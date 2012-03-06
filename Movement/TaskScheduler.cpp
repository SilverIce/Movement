#include "framework/typedefs_p.h"
#include "TaskScheduler.h"

#include "LinkedList.h"

namespace Tasks
{
    namespace detail {}
    using namespace ::Tasks::detail;
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
    class TaskTargetImpl
    {
    public:
        Movement::LinkedList<TaskHandle*> list;

        ~TaskTargetImpl() { assert_state(list.empty()); }
        explicit TaskTargetImpl() {}
    };

    static_assert(sizeof(TaskTargetImpl) <= sizeof(TaskTarget), "");

    inline TaskTargetImpl& getImpl(TaskTarget& target) {
        return (TaskTargetImpl&)target;
    }
    inline const TaskTargetImpl& getImpl(const TaskTarget& target) {
        return (TaskTargetImpl&)target;
    }
}
}

#define ForEach(element, _array, action) { \
        size_t CONCAT(ForEach_iter, __LINE__) = 0; \
        size_t CONCAT(ForEach_end, __LINE__) = (_array).size(); \
        while (CONCAT(ForEach_iter, __LINE__) < CONCAT(ForEach_end, __LINE__)) { \
            element = (_array)[CONCAT(ForEach_iter, __LINE__)]; \
            ++CONCAT(ForEach_iter, __LINE__); \
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
#include "TaskScheduler.Tests.hpp"

namespace Tasks
{
    class TaskExecutorImpl : public
        //TaskExecutorImpl_VectorPOD110
        TaskExecutorImpl_LinkedList110
    {
        friend class TaskExecutor;
    public:
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

    void TaskExecutor::Update(MSTime time)
    {
        TaskExecutor_Args tt(*this, time);
        impl.Update(tt);
    }

    void TaskExecutor::CancelAllTasks()
    {
        impl.CancelAllTasks();
    }

    //////////////////////////////////////////////////////////////////////////

    TaskTarget::TaskTarget() {
        getImpl(*this).TaskTargetImpl::TaskTargetImpl();
    }

    TaskTarget::~TaskTarget() {
        getImpl(*this).~TaskTargetImpl();
    }

    bool TaskTarget::hasTaskAttached() const {
        return !getImpl(*this).list.empty();
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
        m_executor->AddTask(task, exec_time, m_objectId);
    }
}
