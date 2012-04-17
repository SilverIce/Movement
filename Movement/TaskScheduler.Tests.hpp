#include "framework/RdtscTimer.h"
#include "framework/gtest.h"
#include <typeinfo>
#include <map>
#include <set>

namespace Tasks { namespace detail
{
    TEST(ReferenceCountable, basic)
    {
        struct Obj : public ReferenceCountable {
            bool& deleted;
            Obj(bool& Deleted) : deleted(Deleted) { deleted = false;}
            ~Obj() { deleted = true;}
        };

        {
            Reference<ReferenceCountable> ref;
            EXPECT_TRUE(!ref.pointer());
        }

        typedef Reference<Obj> ObjRef;

        {
            bool deleted;
            ObjRef * ref = new ObjRef(new Obj(deleted));
            EXPECT_TRUE(ref->pointer());
            EXPECT_TRUE(!deleted);

            delete ref;
            EXPECT_TRUE(deleted);
        }
        {
            bool deleted;
            ObjRef * ref = new ObjRef(new Obj(deleted));
            EXPECT_TRUE(ref->pointer());
            EXPECT_TRUE(!deleted);

            ObjRef * ref2 = new ObjRef(*ref);
            EXPECT_TRUE(ref2->pointer());
            EXPECT_TRUE(!deleted);
            EXPECT_TRUE(ref2->pointer() == ref->pointer());

            delete ref;
            EXPECT_TRUE(!deleted);
            EXPECT_TRUE(ref2->pointer());

            delete ref2;
            EXPECT_TRUE(deleted);
        }
    }

    #pragma region performance test
    enum{
        exec_delay_min = 100,   //ms
        exec_delay_max = 50 * 1000,  //ms
    };

    enum Preferences{
        TaskOwnerBalance = 450,
        GrowPerTick = 50,
        TasksPerOwner = 10,
        TaskOwnerInitial = TaskOwnerBalance - GrowPerTick,

        UpdateTicksAmount = 10 * 1000,
        PseudoSleepTime = 100,
    };

    class ITaskExecutor2 : public ITaskExecutor
    {
    public:
        virtual bool HasCallBacks() const = 0;
        virtual uint64 summaryTicks() const = 0;
        virtual void reportTest() = 0;

        const char* name;

        RdtscTimer timerAddTask;
        RdtscTimer timerCancelTask;
        RdtscTimer timerUpdate;

        std::set<TaskTarget*> taskList;
    };

    template<class Impl>
    class taskExecutor : public ITaskExecutor2
    {
        Impl& impl;
        int32 m_objectsRegistered;
        MSTime updateCounter;

        NON_COPYABLE(taskExecutor);
    public:

        typedef Impl IMPL;

        taskExecutor() : impl(*new Impl()), m_objectsRegistered(0) {
            name = typeid(Impl).name();
        }

        ~taskExecutor() {
            delete &impl;
        }

        void AddTask(ICallBack * task, MSTime exec_time, TaskTarget* ownerId) override
        {
            if (ownerId && !ownerId->hasTaskAttached())
                Register(*ownerId);

            if (timerUpdate.InProgress()) {
                RdtscInterrupt in(timerUpdate);
                RdtscCall c(timerAddTask);
                impl.AddTask(task, exec_time, ownerId);
            }
            else {
                RdtscCall c(timerAddTask);
                impl.AddTask(task, exec_time, ownerId);
            }
        }

        void Execute(MSTime time) override
        {
            RdtscCall c(timerUpdate);
            TaskExecutor_Args tt(*this, time, updateCounter.time);
            impl.Execute(tt);
            updateCounter += 1;
        }

        void Register(TaskTarget& obj)
        {
            taskList.insert(&obj);
        }

        void CancelTasks(TaskTarget& obj) override
        {
            taskList.erase(&obj);
            RdtscCall c(timerCancelTask);
            impl.CancelTasks(obj);
        }

        bool HasCallBacks() const override {
            return impl.hasCallbacks();
        }

        uint64 summaryTicks() const override {
            return timerAddTask.passed()+timerCancelTask.passed()+timerUpdate.passed();
        }

        void reportTest()
        {
//#define WRITET(timer) " avg _count %I64d %I64d %I64d", timer.passed(), timer.avg(), timer._count());
#define WRITET(timer_lowerbound) " avg count %u %I64d", timer_lowerbound.avg(), timer_lowerbound.count());

            log_console(" ======= %s ======= ", typeid(Impl).name());
            log_console("timerAddTask   " WRITET(timerAddTask);
            log_console("timerUpdate    " WRITET(timerUpdate);
            log_console("timerCancelTask" WRITET(timerCancelTask);
            log_console("summary ticks  %I64d", summaryTicks());
            //impl.printStats();
            log_console("");
        }
    };

    template<class T, class Y>
    void compare(const taskExecutor<T>& a, const taskExecutor<Y>& b)
    {
        log_console(" ======= %s VS %s ======= ", typeid(T).name(), typeid(Y).name());
        log_console("timerAddTask    %I64d", timerAddTask.passed() );
        log_console("timerUpdate     %I64d", timerUpdate.passed() );
        log_console("timerCancelTask %I64d", timerCancelTask.passed() );
    }

    void produceExecutors(std::vector<ITaskExecutor2*>& executors) {
        ITaskExecutor2* exec[] = {
            new taskExecutor<TaskExecutorImpl_LinkedList110>,
            //new taskExecutor<TaskExecutorImpl_LinkedList111>,
        };
        executors.assign(exec, exec + CountOf(exec));
    }

    struct DoNothingTask : public ICallBack {
        uint32 period;
        myAdress adr;
        char data[60];

        explicit DoNothingTask() {
            period = exec_delay_min + rand() % (exec_delay_max - exec_delay_min);
        }
        explicit DoNothingTask(uint32 _period) {
            period = _period;
        }

        void Execute(TaskExecutor_Args& a) override {
            adr();
            RescheduleTaskWithDelay(a, period);
        }
    };

    struct TT
    {
        uint32 m_ticksCount;
        MSTime lastUpdate;
        ITaskExecutor2* executor;
        uint32 owners_spawned;
        bool pushed;

        explicit TT(ITaskExecutor2& Executor) : m_ticksCount(0), owners_spawned(0)
        {
            executor = &Executor;
            PushTaskOwners(TaskOwnerInitial);
        }

        ~TT()
        {
            executor->reportTest();
            /*log_console(" ======= summary statistics ======= ");
            uint32 ii = 0;
            for (std::multimap<uint64,const  char*>::iterator it = stats.begin(); it!=stats.end(); ++it, ++ii)
                log_console("%u. %I64d %s", ii, it->first, it->second);*/

            while (!executor->taskList.empty()) {
                TaskTarget * targ = *executor->taskList.begin();
                executor->CancelTasks(*targ);
                delete targ;
            }
        }

        void PushTaskOwners(uint32 amount)
        {
            while(amount-- > 0)
            {
                ++owners_spawned;

                DoNothingTask t;
                TaskTarget* target = new TaskTarget();
                uint32 taskCount = TasksPerOwner;
                while(taskCount-- > 0)
                    executor->AddTask(new DoNothingTask(t.period), lastUpdate + t.period, target );
            }
        }

        void CancelTaskOwners(uint32 amount)
        {
            while((amount-- > 0) /*&& !owners.empty()*/) {
                std::set<TaskTarget*>& owners = executor->taskList;
                if (owners.empty())
                    continue;

                uint32 randomIdx = rand() % owners.size();
                std::set<TaskTarget*>::iterator it = owners.begin();
                while(randomIdx-- > 0)
                    ++it;

                TaskTarget& target = **it;
                executor->CancelTasks(target);
                delete &target;
                --owners_spawned;
            }
        }

        bool Execute(MSTime timeNow)
        {
            lastUpdate = timeNow;
            executor->Execute(timeNow);
            ++m_ticksCount;

            //if (!(m_ticksCount % 2))
            {
                if (owners_spawned < TaskOwnerBalance)
                {
                    PushTaskOwners(GrowPerTick);
                }
                else
                {
                    CancelTaskOwners(GrowPerTick);
                }
            }

            return (m_ticksCount < UpdateTicksAmount);
        }
    };

    #pragma endregion

    void testExecutors(void (*testExecutorFn)(ITaskExecutor2&))
    {
        std::vector<ITaskExecutor2*> exec;
        produceExecutors(exec);

        EXPECT_TRUE( !exec.empty() );

        for(uint32 i = 0; i < exec.size(); ++i)
            testExecutorFn(*exec[i]);
        for(uint32 i = 0; i < exec.size(); ++i)
            delete exec[i];
    }

    void printExecutorName(ITaskExecutor2& ex){
        log_console("%s", typeid(ex).name());
    }
    TEST(TaskExecutorTest, printExecutors) {
        log_console("going to test following executors:");
        testExecutors(&printExecutorName);
    }

    void TaskExecutorTest_basicTest(ITaskExecutor2& executor)
    {
        TaskTarget target;
        EXPECT_TRUE( !target.hasTaskAttached() );

        executor.AddTask(new DoNothingTask, 0, &target);
        EXPECT_TRUE( target.hasTaskAttached() );

        executor.CancelTasks(target);
        EXPECT_TRUE( !target.hasTaskAttached() );
    }

    TEST(TaskExecutorTest, basicTest)
    {
        testExecutors(&TaskExecutorTest_basicTest);
    }

    void TaskExecutorTest_basicTest4(ITaskExecutor2& executor)
    {
        struct CallsInfo
        {
            CallsInfo(){
                executed = false;
                deleteCalled = false;
                callsCount = 0;
            }
            bool executed;
            bool deleteCalled;
            int callsCount;
        };

        struct Fake : public ICallBack {
            CallsInfo& inf;

            Fake(CallsInfo& info) : inf(info) {}

            ~Fake() {
                inf.deleteCalled = true;
                ++inf.callsCount;
            }

            void Execute(TaskExecutor_Args& args){
                EXPECT_TRUE( !inf.deleteCalled );
                inf.executed = true;
                ++inf.callsCount;
            }
        };

        {
            CallsInfo info;
            TaskTarget target;
            executor.AddTask(new Fake(info), 10, &target);

            executor.Execute(9);
            EXPECT_TRUE( !info.executed && info.callsCount == 0 );

            executor.Execute(10);
            EXPECT_TRUE( info.executed );
            EXPECT_TRUE( info.callsCount == 2 );
            EXPECT_TRUE( info.deleteCalled );
            executor.CancelTasks(target);
        }

        {
            CallsInfo info;
            TaskTarget target;
            executor.AddTask(new Fake(info), 10, &target);

            executor.CancelTasks(target);
            EXPECT_TRUE( !info.executed && info.callsCount == 1 && info.deleteCalled );

            executor.Execute(10);
            EXPECT_TRUE( !info.executed && info.callsCount == 1 && info.deleteCalled );
            executor.CancelTasks(target);
            EXPECT_TRUE( !info.executed && info.callsCount == 1 && info.deleteCalled );
        }
    }

    TEST(TaskExecutorTest, basicTest4)
    {
        testExecutors(&TaskExecutorTest_basicTest4);
    }

    void TaskExecutorTest_pulseTest(ITaskExecutor2& executor)
    {
        enum {
            MarkTaskPeriod = 900,
            MarksAmount = 6,
        };

        struct Fake : public ICallBack {
            MSTime lastUpdate;
            bool& allowDeleteCall;

            Fake(bool& allowTaskDelete) : allowDeleteCall(allowTaskDelete) {}

            ~Fake() {
                EXPECT_TRUE( allowDeleteCall == true );
            }

            void Execute(TaskExecutor_Args& args) override {
                lastUpdate = args.now;
                RescheduleTaskWithDelay(args, MarkTaskPeriod);
            }
        };

        TaskTarget target;
        Reference<Fake> marks[MarksAmount];
        bool allowTaskDelete = false;

        for (int i = 0; i < CountOf(marks); ++i) {
            marks[i] = new Fake(allowTaskDelete);
            executor.AddTask(marks[i].pointer(), 0, &target);
        }

        int callbacksToSpawn = 1000;
        while (callbacksToSpawn-- > 0) {
            DoNothingTask * task = new DoNothingTask;
            executor.AddTask(task, 0, &target);
        }

        MSTime time = 0;
        while (time < 50000) // 50 seconds
        {
            time += 100;

            executor.Execute(time);
            for (int i = 0; i < CountOf(marks); ++i)
            {
                Fake& mark = *marks[i].pointer();
                EXPECT_TRUE( mark.lastUpdate <= time );
                EXPECT_TRUE( (time - mark.lastUpdate) <= MarkTaskPeriod );      // ensures that task-mark updates regularly
            }
        }
        allowTaskDelete = true;
        executor.CancelTasks(target);
    }

    TEST(TaskExecutorTest, pulse)
    {
        testExecutors(&TaskExecutorTest_pulseTest);
    }

    void TaskExecutorTest_sequenceTest(ITaskExecutor2& executor)
    {
        struct Fake : public ICallBack {
            char symbol;
            char ** textWriteItr;

            void Setup(char Symbol, char ** TextWriteItr) {
                symbol = Symbol;
                textWriteItr = TextWriteItr;
            }

            void Execute(TaskExecutor_Args& args) {
                **textWriteItr = symbol;
                ++(*textWriteItr);
            }
        };

        const char text[] = "ABCDEFGHIKLMN0123456789";
        char text2[CountOf(text)] = {'\0'};
        char * textWriteItr = text2;

        EXPECT_TRUE(memcmp(text, text2, sizeof(text)) != 0);

        TaskTarget target;

        MSTime execTime = 14000;
        for (int i = 0; i < CountOf(text); ++i) {
            Fake * mark = new Fake();
            mark->Setup(text[i], &textWriteItr);
            executor.AddTask(mark, execTime, &target);
            execTime += rand() % 3;
        }

        executor.Execute(execTime);
        EXPECT_TRUE(memcmp(text, text2, sizeof(text)) == 0);
    }

    TEST(TaskExecutorTest, sequenceTest)
    {
        testExecutors(&TaskExecutorTest_sequenceTest);
    }

    void TaskExecutorTest_UpdateCounter(ITaskExecutor2& executor)
    {
        struct Task : public ICallBack {
            uint32 updateCount;
            explicit Task() : updateCount(0) {}
            void Execute(TaskExecutor_Args& args) {
                RescheduleTask(args, args.now+1);
                EXPECT_TRUE(updateCount == args.execTickCount);
                ++updateCount;
            }
        };
        TaskTarget target;
        executor.AddTask(new Task, 0, &target);

        for (uint32 updateCount = 0; updateCount < 4; ++updateCount)
            executor.Execute(updateCount);

        executor.CancelTasks(target);
    }
    TEST(TaskExecutorTest, UpdateCounter) {
        testExecutors(&TaskExecutorTest_UpdateCounter);
    }

    void TaskExecutorTest_performance(ITaskExecutor2& ex) {
        TT tester(ex);
        MSTime time;
        do {
            time += PseudoSleepTime;
        }
        while(tester.Execute(time));
    }

    TEST_DISABLED(TaskExecutorTest, performance) {
        testExecutors(TaskExecutorTest_performance);
    }
}
}
