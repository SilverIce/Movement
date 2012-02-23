#include "framework/typedefs_p.h"
#include "TaskScheduler.h"

namespace Tasks
{
    typedef Movement::counter<ObjectId, ~ObjectId(0)> ObjectCounter;

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

    class TaskHandle;
    class TaskTargetImpl
    {
    public:
        Movement::LinkedList<TaskHandle*> list;
        bool registered;

        bool isRegistered() const { return registered;}

        ~TaskTargetImpl() { mov_assert(!isRegistered()); }
        explicit TaskTargetImpl() : registered(false) {}
    };

    static_assert(sizeof(TaskTargetImpl) <= sizeof(TaskTarget), "");

    inline TaskTargetImpl& getImpl(TaskTarget& target) {
        return (TaskTargetImpl&)target;
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
#include <hash_map>
#include <hash_set>
#include "POD_Arrays.h"
#include "LinkedList.h"

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
        ObjectCounter counter;
    };

    TaskExecutor::TaskExecutor() : impl(*new TaskExecutorImpl()), m_objectsRegistered(0) {}
    TaskExecutor::~TaskExecutor() { delete &impl;}

    void TaskExecutor::AddTask(ICallBack * task, MSTime exec_time, TaskTarget& ownerId)
    {
        assert_state(task);
        if (!ownerId.isRegistered())
            Register(ownerId);
        impl.AddTask(task, exec_time, ownerId);
    }

    void TaskExecutor::CancelTasks(TaskTarget& obj)
    {
        impl.CancelTasks(obj);
    }

    void TaskExecutor::Register(TaskTarget& obj)
    {
        assert_state_msg(!obj.isRegistered(), "object is already registered somewhere");
        ++m_objectsRegistered;
        impl.RegisterObject(obj);
        getImpl(obj).registered = true;
    }

    void TaskExecutor::Unregister(TaskTarget& obj)
    {
        if (!obj.isRegistered())
            return;

        assert_state(m_objectsRegistered > 0);
        --m_objectsRegistered;
        impl.RemoveObject(obj);
        getImpl(obj).registered = false;
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

    bool TaskTarget::isRegistered() const {
        return ((TaskTargetImpl*)this)->registered;
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
            m_executor->Unregister(m_objectId);
        }
        m_executor = NULL;
    }

    void TaskTarget_DEV::CancelTasks()
    {
        if (isRegistered()) {
            mov_assert(m_executor);
            m_executor->CancelTasks(m_objectId);
        }
    }

    void TaskTarget_DEV::AddTask(ICallBack * task, MSTime exec_time)
    {
        mov_assert(m_executor);
        m_executor->AddTask(task, exec_time, m_objectId);
    }
}
