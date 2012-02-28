#include "framework/RdtscTimer.h"
#include "framework/gtest.h"
#include <typeinfo>
#include <map>
#include <set>

#include "POD_Array.Tests.hpp"

namespace Tasks { namespace detail
{
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

        NON_COPYABLE(taskExecutor);
    public:

        typedef Impl IMPL;

        taskExecutor() : impl(*new Impl()), m_objectsRegistered(0) {
            name = typeid(Impl).name();
        }

        ~taskExecutor() {
            delete &impl;
        }

        void AddTask(ICallBack * task, MSTime exec_time, TaskTarget& ownerId) override
        {
            if (!ownerId.hasTaskAttached())
                Register(ownerId);

            if (timerUpdate.InProgress()) {
                RdtscInterrupt in(timerUpdate);
                RdtscCall c(timerAddTask);
                impl.AddTask(task, exec_time, ownerId) ;
            }
            else {
                RdtscCall c(timerAddTask);
                impl.AddTask(task, exec_time, ownerId) ;
            }
        }

        void Update(MSTime time) override
        {
            RdtscCall c(timerUpdate);
            TaskTarget target;
            TaskExecutor_Args tt(*this, time);
            impl.Update(tt);
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
            //new taskExecutor<TaskExecutorImpl_VectorPendingPODWrong111>,
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
        std::vector<ITaskExecutor2*> executors;
        uint32 owners_spawned;
        bool pushed;

        TT() : m_ticksCount(0), owners_spawned(0)
        {
            produceExecutors(executors);
            PushTaskOwners(TaskOwnerInitial);
        }

        ~TT()
        {
            std::multimap<uint64, const char*> stats;
            ForEach(ITaskExecutor2* elem, executors, {
                stats.insert(std::multimap<uint64, const char*>::value_type(elem->summaryTicks(),elem->name));
                elem->reportTest();
            });
            log_console(" ======= summary statistics ======= ");
            uint32 ii = 0;
            for (std::multimap<uint64,const  char*>::iterator it = stats.begin(); it!=stats.end(); ++it, ++ii)
                log_console("%u. %I64d %s", ii, it->first, it->second);

            ForEach(ITaskExecutor2* elem, executors, {
                while (!elem->taskList.empty()) {
                    TaskTarget * targ = *elem->taskList.begin();
                    elem->CancelTasks(*targ);
                    delete targ;
                }
            });

            ForEach(ITaskExecutor2* elem, executors, delete elem);
        }

        void PushTaskOwners(uint32 amount)
        {
            while(amount-- > 0)
            {
                ++owners_spawned;

                DoNothingTask t;

                ForEach(ITaskExecutor2 * ex, executors, {
                    TaskTarget* target = new TaskTarget();
                    uint32 taskCount = TasksPerOwner;
                    while(taskCount-- > 0)
                        ex->AddTask(new DoNothingTask(t.period), lastUpdate + t.period, *target );
                });
            }
        }

        void CancelTaskOwners(uint32 amount)
        {
            while((amount-- > 0) /*&& !owners.empty()*/) {
                ForEach(ITaskExecutor2 * ex, executors, {
                    std::set<TaskTarget*>& owners = ex->taskList;
                    if (owners.empty())
                        continue;

                    uint32 randomIdx = rand() % owners.size();
                    std::set<TaskTarget*>::iterator it = owners.begin();
                    while(randomIdx-- > 0)
                        ++it;

                    TaskTarget& target = **it;
                    ex->CancelTasks(target);
                    delete &target;
                });
                --owners_spawned;
            }
        }

        bool Execute(MSTime timeNow)
        {
            lastUpdate = timeNow;
            ForEach(ITaskExecutor2 * ex, executors, {
                ex->Update(timeNow);
            });
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

    TEST(TaskExecutorTest, performance)
    {
        TT tester;
        MSTime time;
        do {
            time += PseudoSleepTime;
        }
        while(tester.Execute(time));
    }

    #pragma endregion

    void testExecutors(void (*testExecutorFn)(ITaskExecutor2&))
    {
        ITaskExecutor2* exec[] = {
            //new taskExecutor<TaskExecutorImpl_VectorPOD110>,
            new taskExecutor<TaskExecutorImpl_LinkedList110>,
            //new taskExecutor<TaskExecutorImpl_LinkedList111>,
            //new taskExecutor<TaskExecutorImpl_VectorPendingPODWrong111>,
            //new taskExecutor<TaskExecutorImpl_LinkedList112>,
        };

        EXPECT_TRUE( CountOf(exec) > 0 );

        for(int i = 0; i < CountOf(exec); ++i)
            testExecutorFn(*exec[i]);
        for(int i = 0; i < CountOf(exec); ++i)
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

        executor.AddTask(new DoNothingTask, 0, target);
        EXPECT_TRUE( target.hasTaskAttached() );

        executor.CancelTasks(target);
        EXPECT_TRUE( !target.hasTaskAttached() );

        {
            TaskTarget target2;
            executor.AddTask(new DoNothingTask, 0, target2);
            EXPECT_TRUE( target2.hasTaskAttached() );
            executor.CancelTasks(target2);
        }
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
            Fake * task = new Fake(info);
            TaskTarget target;
            executor.AddTask(task, 10, target);

            executor.Update(9);
            EXPECT_TRUE( !info.executed && info.callsCount == 0 );

            executor.Update(10);
            EXPECT_TRUE( info.executed );
            EXPECT_TRUE( info.callsCount == 2 );
            EXPECT_TRUE( info.deleteCalled );
            executor.CancelTasks(target);
        }

        {
            CallsInfo info;
            Fake * task = new Fake(info);
            TaskTarget target;
            executor.AddTask(task, 10, target);

            executor.CancelTasks(target);
            EXPECT_TRUE( !info.executed && info.callsCount == 1 && info.deleteCalled );

            executor.Update(10);
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
        Fake* marks[MarksAmount];
        bool allowTaskDelete = false;

        for (int i = 0; i < CountOf(marks); ++i) {
            marks[i] = new Fake(allowTaskDelete);
            executor.AddTask(marks[i], 0, target);
        }

        int callbacksToSpawn = 1000;
        while (callbacksToSpawn-- > 0) {
            DoNothingTask * task = new DoNothingTask;
            executor.AddTask(task, 0, target);
        }

        MSTime time = 0;
        while (time < 50000) // 50 seconds
        {
            time += 100;

            executor.Update(time);
            for (int i = 0; i < CountOf(marks); ++i)
            {
                Fake& mark = *marks[i];
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
            int taskId;
            int * nextTaskId;

            void Setup(int & NextTaskId) {
                taskId = NextTaskId;
                ++NextTaskId;
                nextTaskId = &NextTaskId;
            }

            void Execute(TaskExecutor_Args& args) {
                EXPECT_TRUE(taskId == (*nextTaskId));
                ++(*nextTaskId);
            }
        };

        int taskId = 0;
        TaskTarget target;
        Fake marks[20];

        for (int i = 0; i < 20; ++i) {
            Fake * mark = new Fake();
            mark->Setup(taskId);
            executor.AddTask(mark, 14000, target);
        }

        taskId = 0;
        executor.Update(15000);
        executor.CancelTasks(target);
    }

    TEST(TaskExecutorTest, sequenceTest)
    {
        testExecutors(&TaskExecutorTest_sequenceTest);
    }

}
}
