#include "framework/typedefs_p.h"
#include "TaskScheduler.h"

#include "LinkedList.h"

namespace Tasks
{
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
    using Movement::log_write;
    using Movement::log_console;
    using Movement::uint64;

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

    struct TaskTargetImpl : private Movement::LinkedList<TaskHandle*>
    {
        typedef Movement::LinkedList<TaskHandle*> base;
        using base::empty;
        using base::first;
        using base::element_type;

        void link(element_type& node) {
            if (this != (TaskTargetImpl*)&TaskTarget::Null)
                link_last(node);
        }

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

#define ForEach(element, _array, action) { \
        size_t ForEach_iter = 0; \
        size_t ForEach_end = (_array).size(); \
        while (ForEach_iter < ForEach_end) { \
            element = (_array)[ForEach_iter]; \
            ++ForEach_iter; \
            action; \
        } \
    }

#include <vector>
#include <algorithm>
//#include <hash_map>
//#include <hash_set>
#include "POD_Arrays.h"

#include "TaskExecutorImpl_Vector1.10.hpp"
#include "TaskExecutorImpl_LinkedList1.10.hpp"
/*
#include "TaskExecutorImpl_LinkedList1.11.hpp"
#include "TaskExecutorImpl_LinkedList1.13.hpp"*/
#include "POD_Array.Tests.hpp"
#include "TaskScheduler.Tests.hpp"

namespace Tasks
{
    class TaskExecutorImpl : public
        //TaskExecutorImpl_VectorPOD110
        TaskExecutorImpl_LinkedList110
    {
    };

    TaskExecutor::TaskExecutor() : impl(*new TaskExecutorImpl()) {}
    TaskExecutor::~TaskExecutor() { delete &impl;}

    void TaskExecutor::AddTask(ICallBack * task, MSTime exec_time, TaskTarget& ownerId)
    {
        assert_state(task);
        impl.AddTask(task, exec_time, ownerId);
    }

    void TaskExecutor::CancelTasks(TaskTarget& obj)
    {
        impl.CancelTasks(obj);
    }

    void TaskExecutor::Execute(MSTime time)
    {
        TaskExecutor_Args tt(*this, time, m_updateCounter.time);
        impl.Execute(tt);
        m_updateCounter += 1;
    }

    void TaskExecutor::CancelAllTasks()
    {
        impl.CancelAllTasks();
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

    TaskTarget TaskTarget::Null;

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
        m_executor->AddTask(task, exec_time, m_objectId);
    }
}
