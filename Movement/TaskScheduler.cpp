#include "typedefs_p.h"
#include "TaskScheduler.h"

namespace Tasks
{
    typedef Movement::counter<ObjectId, ~ObjectId(0)> ObjectCounter;

    using Movement::log_write;
    using Movement::log_write_trace;
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

        /** Decreases references count by 1. If there are no one who references this callback it sends 'delete' signal
            to functor and deallocates self. */
        void release() {
            adr();
            if ((--ref_count) <= 0)
                delete this;
        }

    protected:

        NON_COPYABLE(CallBack);
        ~CallBack() {
            cleanup();
        }

        void cleanup() {
            adr();
            if (!isCancelled()) {
                (*m_func)(m_functor, 0);    // delete m_functor
                m_functor = 0;
                m_func = &DoNothing;
            }
            adr();
        }

        void * m_functor;
        ExecFunc m_func;
        int32 ref_count;
        myAdress adr;

        static void DoNothing(void*, TaskExecutor_Args*) {}

    public:

        /**  */
        bool isCancelled() const {
            return m_func == &DoNothing;
        }

        /** Decreases references count by 1. If there are no one who references this callback it sends 'delete' signal
            to functor, but do not deallocates self. 'isCancelled' returns true for now */
        void releaseSoft() {
            if ((--ref_count) <= 0)
                cleanup();
        }
    };

    struct NullCallBack : public CallBack
    {
        explicit NullCallBack() : CallBack(0, &DoNothing) {}
    };
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

#include "TaskScheduler.Tests.hpp"

namespace Tasks
{
    class TaskExecutorImpl : public
        //TaskExecutorImpl_VectorPOD110
        TaskExecutorImpl_LinkedList110
        //TaskExecutorImpl_VectorHashPending112  -- that one doesn't destructs the task at task cancelling
    {
        friend class TaskExecutor;
    public:
        ObjectCounter counter;
    };

    CallBack* CallBackPublic(void* functor, ExecFunc execFunc) {
        return new CallBack(functor, execFunc);
    }

    TaskExecutor::TaskExecutor() : impl(*new TaskExecutorImpl()), m_objectsRegistered(0) {}
    TaskExecutor::~TaskExecutor() { delete &impl;}

    void TaskExecutor::AddTask(CallBack * callback, MSTime exec_time, TaskTarget& ownerId)
    {
        if (!ownerId.isRegistered())
            Register(ownerId);
        impl.AddTask(callback, exec_time, ownerId.objectId);
    }

    void TaskExecutor::CancelTasks(const TaskTarget& ownerId)
    {
        impl.CancelTasks(ownerId.objectId);
    }

    void TaskExecutor::Register(TaskTarget& obj)
    {
        assert_state_msg(!obj.isRegistered(), "object is already registered somewhere");
        ++m_objectsRegistered;
        obj.objectId = impl.counter.NewId();
        impl.RegisterObject(obj.objectId);
    }

    void TaskExecutor::Unregister(TaskTarget& obj)
    {
        if (!obj.isRegistered())
            return;

        assert_state(m_objectsRegistered > 0);
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
            m_executor->Unregister(m_objectId);
        }
        m_executor = NULL;
    }

    void TaskTarget_DEV::AddTask(CallBack * callback, MSTime exec_time)
    {
        mov_assert(m_executor);
        m_executor->AddTask(callback, exec_time, m_objectId);
    }
}
