#include "typedefs_p.h"
#include "TaskScheduler.h"

namespace Tasks
{
    typedef Movement::counter<ObjectId, ~ObjectId(0)> ObjectCounter;

    using Movement::log_write;
    using Movement::log_write_trace;
    using Movement::log_console;

#ifdef TASKSCHEDULER_DEBUGGING
    const char* myAdressDeleted = "yep!";
    class myAdress 
    {
        void * const adress;
        const char * deleted;
    public:
        myAdress() : adress(this), deleted("nope") {}
        ~myAdress() { (*this)(); deleted = myAdressDeleted;}
        myAdress(const myAdress& adr) : adress(this), deleted("nope") {}
        myAdress& operator = (const myAdress& adr) { adr(); return *this;}
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

    class CallBack
    {
    public:
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

    CallBack* CallBackPublic(void* functor, ExecFunc execFunc)
    {
        return new CallBack(functor, execFunc);
    }

    /*struct NullCallBack : public CallBack
    {
        explicit NullCallBack() : CallBack(0, &DoNothing) {}
    private:
        static void DoNothing(TaskExecutor_Args*, void*) {}
    };*/
}


#include <vector>
#include <deque>
#include <algorithm>
#include <hash_map>
#include <hash_set>

#include "TaskExecutorImpl_Vector1.10.hpp"
#include "TaskExecutorImpl_VectorHash1.10.hpp"

/*
#include "TaskScheduler.Tests.hpp"

#include "TaskExecutorImpl_Deque.hpp"
#include "TaskExecutorImpl_Vector1.00.hpp"
#include "TaskExecutorImpl_Vector1.05.hpp"
#include "TaskExecutorImpl_Vector1.11.hpp"
#include "TaskExecutorImpl_VectorHash1.00.hpp"
#include "TaskExecutorImpl_VectorHash1.01.hpp"
#include "TaskExecutorImpl_VectorHash1.02.hpp"*/
#include "TaskExecutorImpl_VectorHash1.12.hpp"

namespace Tasks
{
    class TaskExecutorImpl : public 
        //TaskExecutorImpl_Vector110
        TaskExecutorImpl_VectorHashPending112
    {
        friend class TaskExecutor;
    };

    TaskExecutor::TaskExecutor() : impl(*new TaskExecutorImpl()), m_objectsRegistered(0) {}
    TaskExecutor::~TaskExecutor() { delete &impl;}

    void TaskExecutor::AddTask(CallBack * callback, MSTime exec_time, const TaskTarget& ownerId)
    {
        impl.AddTask(callback, exec_time, ownerId.objectId);
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
        impl.RegisterObject(obj.objectId);
    }

    void TaskExecutor::RemoveObject(TaskTarget& obj)
    {
        if (!obj.isRegistered())
            return;
        
        --m_objectsRegistered;
        impl.RemoveObject(obj.objectId);
        obj.objectId = 0;
    }

    void TaskExecutor::Update(MSTime time)
    {
        TaskExecutor_Args tt = {*this, NULL, time, TaskTarget()};
        impl.Update(tt);
        tt.objectId = TaskTarget(); // overwrite it to not fail assertion in TaskTarget destructor
    }

    bool TaskExecutor::HasCallBacks() const
    {
        return !impl.tasks.empty();
    }

    void TaskExecutor::CancelAllTasks()
    {
        impl.CancelAllTasks();
    }

    //////////////////////////////////////////////////////////////////////////

    TaskTarget::~TaskTarget() {
        mov_assert(!isRegistered());
    }

    void TaskTarget_DEV::SetExecutor(ITaskExecutor& executor)
    {
        mov_assert(!m_executor && !isRegistered());
        m_executor = &executor;
    }

    void TaskTarget_DEV::Unregister()
    {
        if (isRegistered()) {
            mov_assert(m_executor);
            m_executor->RemoveObject(m_objectId);
        }
        m_executor = NULL;
    }

    void TaskTarget_DEV::AddTask(CallBack * callback, MSTime exec_time)
    {
        mov_assert(m_executor);
        if (!isRegistered())
            m_executor->RegisterObject(m_objectId);
        m_executor->AddTask(callback, exec_time, m_objectId);
    }
}
