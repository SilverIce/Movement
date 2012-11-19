#include "TaskScheduler.h"

#include "framework/typedefs_p.h"
#include "LinkedList.h"
#include "LinkedListSimple.h"
#include "POD_Arrays.h"

#include <time.h>
#include <algorithm>
#include <QtCore/QMap>
#include <QtCore/QVector>

namespace Tasks
{
    using Movement::log_write;
    using Movement::log_console;
    using Movement::uint64;
    using Movement::int32;
    using Movement::uint32;
    using Movement::MSTime;

    namespace detail {}
    using namespace ::Tasks::detail;

    void ReferenceCountable::release()
    {
        assert_state(refCount > 0); // asserts that already released(deleted) object will not be released again
        --refCount;
        if (refCount == 0)
            delete this;
    }
}

namespace Tasks { namespace detail
{
#ifdef TASKSCHEDULER_DEBUGGING
    const char* myAdressDeleted = "yep!";
    class myAdress
    {
        void * adress;
        const char * deleted;
        void init() {
            adress = this;
            deleted = "nope";
        }
    public:
        myAdress() { init();}
        ~myAdress() { (*this)(); deleted = myAdressDeleted;}
        myAdress(const myAdress& adr) { init();}
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

    typedef ICallBack CallBack;

    class TaskHandle {};

    struct TaskTargetImpl : public Movement::LinkedListSimple<TaskHandle*>
    {
        static inline TaskTargetImpl& cast(TaskTarget& target) {
            return (TaskTargetImpl&)target;
        }
        static inline const TaskTargetImpl& cast(const TaskTarget& target) {
            return (TaskTargetImpl&)target;
        }
    };
    static_assert(sizeof(TaskTargetImpl) <= sizeof(TaskTarget), "");
}
}

#include "TaskExecutorImpl_Vector1.10.hpp"
#include "TaskExecutorImpl_LinkedList1.10.hpp"
/*
#include "TaskExecutorImpl_LinkedList1.11.hpp"
#include "TaskExecutorImpl_LinkedList1.13.hpp"*/
#include "POD_Array.Tests.hpp"
#include "TaskScheduler.Tests.hpp"

namespace Tasks
{
    typedef
        TaskExecutorImpl_LinkedList110
        // some another implementation placeholder
    ImplBase;

    class TaskExecutorImpl : public ImplBase
    {
        MSTime m_updateCounter;
        //MSTime m_updateTimeOffset; // shifts all incoming time values

    public:

        MSTime TickCount;

        void AddTask(ICallBack * task, MSTime exec_time, TaskTarget* target) {
            assert_state(task);
            ImplBase::AddTask(task, exec_time, target);
        }

        void Execute(ITaskExecutor& exec, MSTime time) {
            assert_state(TickCount <= time);
            TickCount = time;
            TaskExecutor_Args args(exec, time, m_updateCounter.time);
            ImplBase::Execute(args);
            m_updateCounter += 1;
        }
    };

    TaskExecutor::TaskExecutor() : impl(*new TaskExecutorImpl()) {}
    TaskExecutor::~TaskExecutor() { delete &impl;}

    void TaskExecutor::AddTask(ICallBack * task, MSTime exec_time, TaskTarget* target)
    {
        impl.AddTask(task, exec_time, target);
    }

    void TaskExecutor::CancelTasks(TaskTarget& obj)
    {
        impl.CancelTasks(obj);
    }

    void TaskExecutor::Execute(MSTime time)
    {
        impl.Execute(*this, time);
    }

    void TaskExecutor::CancelAllTasks()
    {
        impl.CancelAllTasks();
    }

    MSTime TaskExecutor::Time()
    {
        return impl.TickCount;
    }

    //////////////////////////////////////////////////////////////////////////

    TaskTarget::TaskTarget() {
        TaskTargetImpl::cast(*this).TaskTargetImpl::TaskTargetImpl();
    }

    TaskTarget::~TaskTarget() {
        TaskTargetImpl::cast(*this).~TaskTargetImpl();
    }

    bool TaskTarget::hasTaskAttached() const {
        return !TaskTargetImpl::cast(*this).empty();
    }

    void TaskTarget_DEV::SetExecutor(ITaskExecutor& executor)
    {
        mov_assert(!m_executor);
        m_executor = &executor;
    }

    void TaskTarget_DEV::Unregister()
    {
        if (m_executor) {
            m_executor->CancelTasks(m_objectId);
        }
        m_executor = NULL;
    }

    void TaskTarget_DEV::CancelTasks()
    {
        if (m_executor) {
            m_executor->CancelTasks(m_objectId);
        }
    }

    void TaskTarget_DEV::AddTask(ICallBack * task, MSTime exec_time)
    {
        mov_assert(m_executor);
        m_executor->AddTask(task, exec_time, &m_objectId);
    }
}
