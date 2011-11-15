#include "RdtscTimer.h"
#include "gtest/gtest.h"
#include <typeinfo>
#include <map>

#include "POD_Array.Tests.hpp"

namespace Tasks
{
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

    #pragma region details
    class ITaskExecutor2 : public ITaskExecutor
    {
    public:
        virtual void Register(TaskTarget& obj) = 0;
        virtual void Unregister(TaskTarget& obj) = 0;
        virtual bool HasCallBacks() const = 0;
        virtual ~ITaskExecutor2() {}
        virtual uint64 summaryTicks() const = 0;
        virtual void reportTest() = 0;

        template<class T>
        inline void AddTask(T * functor, MSTime exec_time, TaskTarget& ownerId)
        {
            AddTask(CallBackPublic(functor), exec_time, ownerId);
        }

        void AddTask(CallBack * functor, MSTime exec_time, TaskTarget& ownerId) override = 0;

        const char* name;
    };

    template<class Impl>
    class taskExecutor : public ITaskExecutor2
    {
        Impl& impl;
        int32 m_objectsRegistered;
        ObjectCounter m_counter;

        NON_COPYABLE(taskExecutor);
    public:

        typedef Impl IMPL;

        RdtscTimer timerAddTask;
        RdtscTimer timerCancelTask;
        RdtscTimer timerUpdate;

        taskExecutor() : impl(*new Impl()), m_objectsRegistered(0) {
            name = typeid(Impl).name();
        }

        ~taskExecutor() {
            delete &impl;
        }

        void AddTask(CallBack * callback, MSTime exec_time, TaskTarget& ownerId) override
        {
            if (!ownerId.isRegistered())
                Register(ownerId);

            if (timerUpdate.InProgress()) {
                RdtscInterrupt in(timerUpdate);
                RdtscCall c(timerAddTask);
                impl.AddTask(callback, exec_time, ownerId.objectId) ;
            }
            else {
                RdtscCall c(timerAddTask);
                impl.AddTask(callback, exec_time, ownerId.objectId) ;
            }
        }

        void Update(MSTime time) override
        {
            RdtscCall c(timerUpdate);
            TaskExecutor_Args tt = {*this, NULL, time, TaskTarget()};
            impl.Update(tt);
            tt.objectId = TaskTarget();
        }

        void Register(TaskTarget& obj) override
        {
            /*if (obj.isRegistered()){
                log_fatal("object is already registered somewhere");
                return;
            }*/

            obj.objectId = m_counter.NewId();
            impl.RegisterObject(obj.objectId);
        }

        void Unregister(TaskTarget& obj) override
        {
            if (!obj.isRegistered())
                return;

            RdtscCall c(timerCancelTask);
            impl.RemoveObject(obj.objectId);
            obj.objectId = 0;
        }

        void CancelTasks(const TaskTarget& ownerId) override
        {
            impl.CancelTasks(ownerId.objectId);
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
#define WRITET(timer) " avg count %I64d %I64d", timer.avg(), timer.count());

            log_console(" ======= %s ======= ", typeid(Impl).name());
            log_console("timerAddTask   " WRITET(timerAddTask);
            log_console("timerUpdate    " WRITET(timerUpdate);
            log_console("timerCancelTask" WRITET(timerCancelTask);
            log_console("summary ticks  %I64d", summaryTicks());
            impl.printStats();
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
    #pragma endregion

    void produceExecutors(std::vector<ITaskExecutor2*>& executors) {
        ITaskExecutor2* exec[] = {
            new taskExecutor<TaskExecutorImpl_VectorPOD110>,
            //new taskExecutor<TaskExecutorImpl_VectorPendingPODWrong111>,
            new taskExecutor<TaskExecutorImpl_LinkedList110>,
            //new taskExecutor<TaskExecutorImpl_LinkedList111>,
        };
        executors.assign(exec, exec + CountOf(exec));
    }

    #pragma region details

    struct DoNothingTask : Executor<DoNothingTask,true> {
        uint32 period;
        myAdress adr;
        char data[60];

        DoNothingTask() {
            period = exec_delay_min + rand() % (exec_delay_max - exec_delay_min);
        }

        void Execute(TaskExecutor_Args& a) {
            adr();
            a.executor.AddTask(a.callback, a.now + period, a.objectId);
        }
    };

    struct TT : Executor<TT,true>
    {
        uint32 m_ticksCount;
        MSTime lastUpdate;
        std::vector<ITaskExecutor2*> executors;
        std::vector<TaskTarget*> owners;
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
                delete elem;
            });
            log_console(" ======= summary statistics ======= ");
            uint32 ii = 0;
            for (std::multimap<uint64,const  char*>::iterator it = stats.begin(); it!=stats.end(); ++it, ++ii)
                log_console("%u. %I64d %s", ii, it->first, it->second);

            ForEach(TaskTarget* elem, owners, {
                *elem = TaskTarget();
                delete elem;
            });
        }

        void PushTaskOwners(uint32 amount)
        {
            while(amount-- > 0)
            {
                TaskTarget* target = new TaskTarget();
                ForEach(ITaskExecutor2 * ex, executors, {
                    ex->Register(*target);
                });
                owners.push_back(target);
                ++owners_spawned;

                uint32 taskCount = TasksPerOwner;
                while(taskCount-- > 0) {
                    DoNothingTask t;
                    ForEach(ITaskExecutor2 * ex, executors, {
                        ex->AddTask( CallBackPublic(new DoNothingTask(t)), lastUpdate + t.period, *target );
                    });
                }
            }
        }

        void CancelTaskOwners(uint32 amount)
        {
            while((amount-- > 0) && !owners.empty()) {
                std::vector<TaskTarget*>::iterator target = owners.begin() + (rand() % owners.size());
                ForEach(ITaskExecutor2 * ex, executors, {
                    TaskTarget tt(**target);
                    ex->Unregister(tt);
                    tt = TaskTarget();
                });
                **target = TaskTarget();
                delete (*target);
                owners.erase(target);
                --owners_spawned;
            }
        }

        void Execute(TaskExecutor_Args& args)
        {
            lastUpdate = args.now;
            ForEach(ITaskExecutor2 * ex, executors, {
                ex->Update(args.now);
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

            if (m_ticksCount < UpdateTicksAmount && !owners.empty())
                args.executor.AddTask(args.callback, args.now + 1, args.objectId);
        }
    };
#pragma endregion

    void testExecutors(void (*testExecutorFn)(ITaskExecutor2&))
    {
        ITaskExecutor2* exec[] = {
            new taskExecutor<TaskExecutorImpl_VectorPOD110>,
            new taskExecutor<TaskExecutorImpl_LinkedList110>,
            //new taskExecutor<TaskExecutorImpl_LinkedList111>,
            //new taskExecutor<TaskExecutorImpl_VectorPendingPODWrong111>,
            //new taskExecutor<TaskExecutorImpl_LinkedList112>,
        };

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

    TEST(CallbackTest, basic)
    {
        struct Fake : Executor<Fake,false> {
            bool executed;
            bool deleteCalled;
            int callsCount;
            Fake() :
                executed(false),
                deleteCalled(false),
                callsCount(0)
            {}

            void Delete() {
                deleteCalled = true;
                ++callsCount;
            }

            static void Static_Execute(void* _me, TaskExecutor_Args* args){
                if (args)
                    ((Fake*)_me)->Execute();
                else
                    ((Fake*)_me)->Delete();
            }

            void Execute() {
                executed = true;
                ++callsCount;
            }
        };

        {
            Fake fake;
            CallBack& callback = *CallBackPublic(&fake, &Fake::Static_Execute);
            EXPECT_TRUE( !callback.isCancelled() );

            callback.addref();
            callback.execute(*(TaskExecutor_Args*)0x4);
            EXPECT_TRUE( fake.executed && fake.callsCount == 1);

            callback.releaseSoft();
            EXPECT_TRUE( fake.deleteCalled && fake.callsCount == 2);

            callback.release();
            EXPECT_TRUE( fake.callsCount == 2);
        }
        {
            Fake fake;
            CallBack& callback = *CallBackPublic(&fake, &Fake::Static_Execute);
            EXPECT_TRUE( !callback.isCancelled() );

            callback.addref();
            callback.execute(*(TaskExecutor_Args*)0x4);
            EXPECT_TRUE( fake.executed && fake.callsCount == 1);

            callback.release();
            EXPECT_TRUE( fake.deleteCalled && fake.callsCount == 2);
        }
    }

    void TaskExecutorTest_basicTest(ITaskExecutor2& executor)
    {
        TaskTarget target;
        EXPECT_TRUE( !target.isRegistered() );
        EXPECT_TRUE( !executor.HasCallBacks() );

        executor.Register(target);
        EXPECT_TRUE( target.isRegistered() );
        EXPECT_TRUE( !executor.HasCallBacks() );

        executor.AddTask(new DoNothingTask, 0, target);
        EXPECT_TRUE( executor.HasCallBacks() );

        executor.CancelTasks(target);
        EXPECT_TRUE( !executor.HasCallBacks() );

        executor.Unregister(target);
        EXPECT_TRUE( !target.isRegistered() );

        {
            TaskTarget target2;
            executor.AddTask(new DoNothingTask, 0, target2);
            EXPECT_TRUE( target2.isRegistered() );
            executor.Unregister(target2);
        }
    }

    TEST(TaskExecutorTest, basicTest)
    {
        testExecutors(&TaskExecutorTest_basicTest);
    }

    void TaskExecutorTest_basicTest2(ITaskExecutor2& executor)
    {
        TaskTarget target;

        EXPECT_TRUE( !target.isRegistered() );
        executor.Register(target);
        EXPECT_TRUE( target.isRegistered() );

        executor.AddTask(new DoNothingTask, 0, target);
        EXPECT_TRUE( executor.HasCallBacks() );

        executor.Unregister(target);
        EXPECT_TRUE( !target.isRegistered() );
        EXPECT_TRUE( !executor.HasCallBacks() );
    }

    TEST(TaskExecutorTest, basicTest2)
    {
        testExecutors(&TaskExecutorTest_basicTest2);
    }

    void TaskExecutorTest_basicTest4(ITaskExecutor2& executor)
    {
        struct Fake {
            bool executed;
            bool deleteCalled;
            int callsCount;
            Fake() :
            executed(false),
                deleteCalled(false),
                callsCount(0)
            {}

            void Delete() {
                deleteCalled = true;
                ++callsCount;
            }

            static void Static_Execute(void* _me, TaskExecutor_Args* _args){
                if (_args)
                    ((Fake*)_me)->Execute();
                else
                    ((Fake*)_me)->Delete();
            }

            void Execute() {
                EXPECT_TRUE( !deleteCalled );
                executed = true;
                ++callsCount;
            }
        };

        TaskTarget target;
        {
            Fake task;
            executor.AddTask(&task, 10, target);
            EXPECT_TRUE( executor.HasCallBacks() );

            executor.Update(9);
            EXPECT_TRUE( !task.executed && task.callsCount == 0 );

            executor.Update(10);
            EXPECT_TRUE( task.executed );
            EXPECT_TRUE( task.callsCount == 2 );
            EXPECT_TRUE( task.deleteCalled );
            executor.Unregister(target);
        }

        {
            Fake task;
            executor.AddTask(&task, 10, target);
            EXPECT_TRUE( executor.HasCallBacks() );

            executor.CancelTasks(target);
            EXPECT_TRUE( !task.executed && task.callsCount == 1 && task.deleteCalled );

            executor.Update(10);
            EXPECT_TRUE( !task.executed && task.callsCount == 1 && task.deleteCalled );
            executor.Unregister(target);
            EXPECT_TRUE( !task.executed && task.callsCount == 1 && task.deleteCalled );
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
        };
        struct Fake {
            bool deleteCalled;
            MSTime lastUpdate;
            Fake() : deleteCalled(false) {}

            void Delete() {
                deleteCalled = true;
            }

            static void Static_Execute(void* _me, TaskExecutor_Args* _args){
                if (_args)
                    ((Fake*)_me)->Execute(*_args);
                else
                    ((Fake*)_me)->Delete();
            }

            void Execute(TaskExecutor_Args& args) {
                lastUpdate = args.now;
                args.executor.AddTask(args.callback, args.now + MarkTaskPeriod, args.objectId);
            }
        };

        TaskTarget target;
        Fake marks[6];

        for (int i = 0; i < CountOf(marks); ++i)
            executor.AddTask(&marks[i], 0, target);

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
                Fake& mark = marks[i];
                EXPECT_TRUE( !mark.deleteCalled );      // ensures that task-mark wasn't cancelled
                EXPECT_TRUE( mark.lastUpdate <= time );
                EXPECT_TRUE( (time - mark.lastUpdate) <= MarkTaskPeriod );      // ensures that task-mark updates regularly
            }
        }
        executor.Unregister(target);
    }

    TEST(TaskExecutorTest, pulse)
    {
        testExecutors(&TaskExecutorTest_pulseTest);
    }

    void TaskExecutorTest_sequenceTest(ITaskExecutor2& executor)
    {
        struct Fake {
            int taskId;
            int * nextTaskId;

            void Setup(int & NextTaskId) {
                taskId = NextTaskId;
                ++NextTaskId;
                nextTaskId = &NextTaskId;
            }

            static void Static_Execute(void* _me, TaskExecutor_Args* _args){
                if (_args)
                    ((Fake*)_me)->Execute(*_args);
            }

            void Execute(TaskExecutor_Args& args) {
                EXPECT_TRUE(taskId == (*nextTaskId));
                ++(*nextTaskId);
            }
        };

        int taskId = 0;
        TaskTarget target;
        Fake marks[20];
        for (int i = 0; i < CountOf(marks); ++i) {
            marks[i].Setup(taskId);
            executor.AddTask(&marks[i], 14000, target);
        }

        taskId = 0;
        executor.Update(15000);
        executor.Unregister(target);
    }

    TEST(TaskExecutorTest, sequenceTest)
    {
        testExecutors(&TaskExecutorTest_sequenceTest);
    }

    TEST(TaskExecutorTest, performance)
    {
        taskExecutor<TaskExecutorImpl_VectorPOD110> executor;
        TaskTarget target;
        executor.Register(target);
        executor.AddTask(CallBackPublic(new TT), 0, target);

        MSTime time_;
        while(executor.HasCallBacks()) {
            time_ += PseudoSleepTime;
            executor.Update(time_);
        }
        executor.Unregister(target);
    }
}
