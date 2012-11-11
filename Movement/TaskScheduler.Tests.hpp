#include "framework/RdtscTimer.h"
#include "framework/gtest.h"
#include <typeinfo>
#include <QtCore/QSet>

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

        QSet<TaskTarget*> taskList;
    };

    template<class Impl>
    class taskExecutor : public ITaskExecutor2
    {
        Impl& impl;
        int32 m_objectsRegistered;
        MSTime updateCounter;
        MSTime tickCount;

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
            assert_state(tickCount <= time);
            tickCount = time;
            RdtscCall c(timerUpdate);
            TaskExecutor_Args tt(*this, time, updateCounter.time);
            impl.Execute(tt);
            updateCounter += 1;
        }

        MSTime Time() override {
            return tickCount;
        }

        void Register(TaskTarget& obj)
        {
            taskList.insert(&obj);
        }

        void CancelTasks(TaskTarget& obj) override
        {
            taskList.remove(&obj);
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

    void produceExecutors(QVector<ITaskExecutor2*>& executors) {
        executors
            << new taskExecutor<TaskExecutorImpl_LinkedList110>
        ;
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
                QSet<TaskTarget*>& owners = executor->taskList;
                if (owners.empty())
                    continue;

                uint32 randomIdx = rand() % owners.size();
                QSet<TaskTarget*>::iterator it = owners.begin();
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

    void testExecutors(testing::State& testState, void (*testExecutorFn)(testing::State&,ITaskExecutor2&))
    {
        QVector<ITaskExecutor2*> exec;
        produceExecutors(exec);

        EXPECT_TRUE( !exec.empty() );

        for(int i = 0; i < exec.size(); ++i)
            testExecutorFn(testState, *exec[i]);
        qDeleteAll(exec);
    }

    void printExecutorName(testing::State& testState, ITaskExecutor2& ex){
        log_console("%s", typeid(ex).name());
    }
    TEST(TaskExecutorTest, printExecutors) {
        log_console("going to test following executors:");
        testExecutors(testState, &printExecutorName);
    }

    void TaskExecutorTest_basicTest(testing::State& testState, ITaskExecutor2& executor)
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
        testExecutors(testState, &TaskExecutorTest_basicTest);
    }

    void TaskExecutorTest_basicTest4(testing::State& testState, ITaskExecutor2& executor)
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
            testing::State& testState;

            Fake(CallsInfo& info, testing::State& state) : inf(info), testState(state) {}

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
            executor.AddTask(new Fake(info,testState), 10, &target);

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
            executor.AddTask(new Fake(info,testState), 10, &target);

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
        testExecutors(testState, &TaskExecutorTest_basicTest4);
    }

    void TaskExecutorTest_pulseTest(testing::State& testState, ITaskExecutor2& executor)
    {
        enum {
            MarkTaskPeriod = 900,
            MarksAmount = 6,
        };

        struct Fake : public ICallBack {
            MSTime lastUpdate;
            bool& allowDeleteCall;
            testing::State& testState;

            Fake(bool& allowTaskDelete, testing::State& state)
                : allowDeleteCall(allowTaskDelete), testState(state) {}

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
            marks[i] = new Fake(allowTaskDelete, testState);
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
        testExecutors(testState, &TaskExecutorTest_pulseTest);
    }

    void TaskExecutorTest_sequenceTest(testing::State& testState, ITaskExecutor2& executor)
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
        testExecutors(testState, &TaskExecutorTest_sequenceTest);
    }

    void TaskExecutorTest_UpdateCounter(testing::State& testState, ITaskExecutor2& executor)
    {
        struct Task : public ICallBack {
            uint32 updateCount;
            testing::State& testState;
            explicit Task(testing::State& state) : updateCount(0), testState(state) {}
            void Execute(TaskExecutor_Args& args) {
                RescheduleTask(args, args.now+1);
                EXPECT_TRUE(updateCount == args.execTickCount);
                ++updateCount;
            }
        };
        TaskTarget target;
        executor.AddTask(new Task(testState), 0, &target);

        for (uint32 updateCount = 0; updateCount < 4; ++updateCount)
            executor.Execute(updateCount);

        executor.CancelTasks(target);
    }
    TEST(TaskExecutorTest, UpdateCounter) {
        testExecutors(testState, &TaskExecutorTest_UpdateCounter);
    }

    void TaskExecutorTest_performance(testing::State& testState, ITaskExecutor2& ex) {
        TT tester(ex);
        MSTime time;
        do {
            time += PseudoSleepTime;
        }
        while(tester.Execute(time));
    }
    TEST_DISABLED(TaskExecutorTest, performance) {
        testExecutors(testState, TaskExecutorTest_performance);
    }

    void TaskExecutorTest_TickCount(testing::State& testState, ITaskExecutor2& ex) {
        EXPECT_TRUE( ex.Time() == 0 );
        const MSTime time = 14000;
        ex.Execute(time);
        EXPECT_TRUE( time == ex.Time() );
        ex.Execute(time);
        EXPECT_TRUE( time == ex.Time() );
    }
    TEST(TaskExecutorTest, TickCount) {
        testExecutors(testState, TaskExecutorTest_TickCount);
    }
}
}
#endif
