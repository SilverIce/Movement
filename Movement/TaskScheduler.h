#pragma once

#include "framework/typedefs.h"
#include "Movement/MSTime.h"

namespace Tasks
{
    using Movement::int32;
    using Movement::uint32;
    using Movement::MSTime;

    class ReferenceCountable
    {
    private:
        int32 refCount;
    public:
        explicit ReferenceCountable() : refCount(0) {}
        virtual ~ReferenceCountable() {}
        void addref() { ++refCount;}
        void release();
    private:
        ReferenceCountable(const ReferenceCountable &);
        ReferenceCountable& operator = (const ReferenceCountable &);
    };

    template<class T> class Reference
    {
    private:
        T * m_pointer;
        void setPointer(T * ptr) {
            if (ptr)
                ptr->ReferenceCountable::addref();    // ensure that T inherits ReferenceCountable
            if (m_pointer)
                m_pointer->ReferenceCountable::release();
            m_pointer = ptr;
        }
    public:
        explicit Reference() : m_pointer(NULL) {}
        explicit Reference(T * ptr) : m_pointer(NULL) { setPointer(ptr);}
        Reference(const Reference& other) : m_pointer(NULL) { setPointer(other.pointer());}
        ~Reference() { setPointer(NULL);}

        T* operator ->() const { return m_pointer;}
        T* pointer() const { return m_pointer;}

        Reference& operator = (const Reference& other) {
            setPointer(other.pointer());
            return *this;
        }

        Reference& operator = (T * ptr) {
            setPointer(ptr);
            return *this;
        }
    };

#define NON_COPYABLE(Class) \
    Class(const Class &); \
    Class& operator = (const Class &);

#define NON_COPYABLE_NO_DEFAULT(Class) \
    NON_COPYABLE(Class) \
    explicit Class(); \

    struct TaskExecutor_Args;
    class TaskExecutor;
    class ITaskExecutor;
    class TaskTarget;
    class ICallBack;

    class ITaskExecutor
    {
    public:
        virtual void AddTask(ICallBack * task, MSTime exec_time, TaskTarget& ownerId) = 0;
        virtual void CancelTasks(TaskTarget& ownerId) = 0;
        virtual void Execute(MSTime time) = 0;
    protected:
        ~ITaskExecutor() {}
    };

    class ICallBack : public ReferenceCountable
    {
    protected:
        explicit ICallBack() {}
    public:
        virtual void Execute(TaskExecutor_Args& args) = 0;
    };

    class EXPORT TaskExecutor : public ITaskExecutor
    {
        class TaskExecutorImpl& impl;
        MSTime m_updateCounter;

        NON_COPYABLE(TaskExecutor);
    public:

        explicit TaskExecutor();
        ~TaskExecutor();

        void AddTask(ICallBack * task, MSTime exec_time, TaskTarget& ownerId) override;
        void CancelTasks(TaskTarget& ownerId) override;
        void CancelAllTasks();

        void Execute(MSTime time) override;
    };

    class TaskTarget
    {
    private:
        char m_fields[8+8];
        NON_COPYABLE(TaskTarget);
    public:
        bool hasTaskAttached() const;
        explicit TaskTarget();
        ~TaskTarget();
    };

    struct TaskExecutor_Args
    {
        explicit TaskExecutor_Args(ITaskExecutor& Executor, MSTime timeNow, uint32 updCount) :
            executor(Executor),
            callback(0),
            objectId(0),
            now(timeNow),
            updateCount(updCount)
        {
        }

        ITaskExecutor& executor;
        ICallBack* callback;
        TaskTarget* objectId;
        const MSTime now;
        const uint32 updateCount;
    };

    inline void RescheduleTaskWithDelay(TaskExecutor_Args& args, int32 delay) {
        args.executor.AddTask(args.callback, args.now + delay, *args.objectId);
    }

    inline void RescheduleTask(TaskExecutor_Args& args, MSTime executionTime) {
        args.executor.AddTask(args.callback, executionTime, *args.objectId);
    }

    class TaskTarget_DEV
    {
    private:
        ITaskExecutor * m_executor;
        TaskTarget m_objectId;
    private:
        NON_COPYABLE(TaskTarget_DEV);
    public:
        bool hasExecutor() const { return m_executor != 0;}
        ITaskExecutor* getExecutor() const { return m_executor;}
        explicit TaskTarget_DEV() : m_executor(0) {}

        void SetExecutor(ITaskExecutor& executor);
        void Unregister();
        void CancelTasks();
        void AddTask(ICallBack * callback, MSTime exec_time);
    };

    /** Tools:
    */

    template<class Class>
    class ITaskP0 : public ICallBack
    {
    public:
        typedef void (Class::*Method)(TaskExecutor_Args&);
        explicit ITaskP0(Class* _class_instance, Method _method) : _obj(_class_instance), _func(_method) {}
        void Execute(TaskExecutor_Args& args) override { (_obj->*_func)(args);}
    private:
        Class*  _obj;
        Method  _func;
    };

    template<class Class> inline ICallBack * NewITaskP0(Class* _class_instance, void (Class::*_method)(TaskExecutor_Args&)) {
        return new ITaskP0<Class>(_class_instance, _method);
    }
}
