#include "TaskScheduler.h"

namespace Tasks
{
    typedef Movement::counter<ObjectId, ~ObjectId(0)> ObjectCounter;

    using Movement::log_write;
    using Movement::log_write_trace;
    using Movement::log_console;

#define TASKSCHEDULER_DEBUGGING 0

#if (TASKSCHEDULER_DEBUGGING)
    const char* myAdressDeleted = "yep!";
    class myAdress 
    {
        void * const adress;
        const char* deleted;
    public:
        myAdress() : adress(this), deleted("nope") {}
        ~myAdress() { (*this)(); deleted = myAdressDeleted;}
        void operator()() const {
            if (this != adress || deleted == myAdressDeleted)
                log_write("memory corruption detected");
        }
    };
#else
    struct myAdress {
        void operator()() const {}
    };
#endif

    struct CallBack : public CallBackPublic
    {
        explicit CallBack(void* functor, ExecFunc execFunc) :
            m_func(execFunc),
            ref_count(0),
            m_functor(functor)
        {}

        void execute(TaskExecutor_Args& args)
        {
            adr();
            (*m_func)( m_functor, &args );
            adr();
        }

        void addref() { adr(); ++ref_count;}
        void release() {
            adr();
            if ((--ref_count) <= 0)
                delete this;
        }

    private:

        NON_COPYABLE(CallBack);
        ~CallBack() {
            adr();
            (*m_func)(m_functor, 0);    // delete m_functor
            m_functor = 0;
            adr();
        }

        void * m_functor;
        ExecFunc m_func;
        int32 ref_count;
    public:
        myAdress adr;
    };

    /*struct NullCallBack : public CallBack
    {
        explicit NullCallBack() : CallBack(0, &DoNothing) {}
    private:
        static void DoNothing(TaskExecutor_Args*, void*) {}
    };*/
    
    static CallBack* Impl(CallBackPublic * pub) { return (CallBack*)pub;}

    CallBackPublic* CallBackPublic::create(void* functor, ExecFunc execFunc)
    {
        return new CallBack(functor, execFunc);
    }
}


#include <vector>
#include <deque>
#include <algorithm>
#include <hash_map>
#include <hash_set>

#include "TaskExecutorImpl_Vector1.10.hpp"
#include "TaskExecutorImpl_VectorHash1.10.hpp"

/*
#include "TaskExecutorImpl_Deque.hpp"
#include "TaskExecutorImpl_Vector1.00.hpp"
#include "TaskExecutorImpl_Vector1.05.hpp"
#include "TaskExecutorImpl_Vector1.11.hpp"
#include "TaskExecutorImpl_VectorHash1.00.hpp"
#include "TaskExecutorImpl_VectorHash1.01.hpp"
#include "TaskExecutorImpl_VectorHash1.02.hpp"

#include "TaskScheduler.Tests.hpp"*/

namespace Tasks
{
    class TaskExecutorImpl : public 
        //TaskExecutorImpl_Deque
        //TaskExecutorImpl_Vector100
        //TaskExecutorImpl_Vector105
        TaskExecutorImpl_Vector110
    {
        friend class TaskExecutor;
    };

    TaskExecutor::TaskExecutor() : impl(*new TaskExecutorImpl()), m_objectsRegistered(0) {}
    TaskExecutor::~TaskExecutor() { delete &impl;}

    void TaskExecutor::AddTask(CallBackPublic * callback, MSTime exec_time, const TaskTarget& ownerId )
    {
        impl.AddTask(Impl(callback), exec_time, ownerId.objectId);
    }

    void TaskExecutor::CancelTasks(const TaskTarget& ownerId)
    {
        impl.CancelTasks(ownerId.objectId);
    }

    void TaskExecutor::RegisterObject(TaskTarget& obj)
    {
        if (obj.isRegistered()){
            log_function("object is already registered somewhere");
            return;
        }

        ++m_objectsRegistered;
        obj.owner = this;
        impl.RegisterObject(obj.objectId);
    }

    void TaskExecutor::RemoveObject(TaskTarget& obj)
    {
        if (!obj.isRegisteredIn(this)) {
            log_function("object wasn't registered or doesn't belongs to current task executor");
            return;
        }

        --m_objectsRegistered;
        obj.owner = NULL;
        impl.RemoveObject(obj.objectId);
    }

    void TaskExecutor::Update(MSTime time)
    {
        TaskExecutor_Args tt = {*this, NULL, time, TaskTarget(this,0)};
        impl.Update(tt);
    }

    bool TaskExecutor::HasCallBacks() const
    {
        return !impl.tasks.empty();
    }

    void TaskExecutor::CancelAllTasks()
    {
        impl.CancelAllTasks();
    }
}
