#pragma once

#include "framework/typedefs.h"
#include "Movement/MSTime.h"

namespace Tasks
{
    using Movement::int32;
    using Movement::uint32;
    using Movement::MSTime;

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
        virtual void Update(MSTime time) = 0;
        virtual void Register(TaskTarget& obj) = 0;
        virtual void Unregister(TaskTarget& obj) = 0;
    protected:
        ~ITaskExecutor() {}
    };

    class ICallBack
    {
    public:
        explicit ICallBack() : refCount(0) {}
        virtual ~ICallBack() {}

        virtual void Execute(TaskExecutor_Args& args) = 0;

        void addref() { ++refCount;}
        void release() {
            if ((--refCount) <= 0)
                delete this;
        }
    private:
        int32 refCount;
    };

    class TaskExecutor : public ITaskExecutor
    {
        class TaskExecutorImpl& impl;
        int32 m_objectsRegistered;

        NON_COPYABLE(TaskExecutor);
    public:

        explicit TaskExecutor();
        ~TaskExecutor();

        void AddTask(ICallBack * task, MSTime exec_time, TaskTarget& ownerId) override;
        void CancelTasks(TaskTarget& ownerId) override;
        void CancelAllTasks();

        void Register(TaskTarget& obj) override;
        void Unregister(TaskTarget& obj) override;

        void Update(MSTime time) override;
    };

    typedef uint32 ObjectId;

    class TaskTarget
    {
    private:
        char m_fields[8+8];
        NON_COPYABLE(TaskTarget);
    public:
        bool isRegistered() const;
        explicit TaskTarget();
        ~TaskTarget();
    };

    struct TaskExecutor_Args
    {
        explicit TaskExecutor_Args(ITaskExecutor& Executor, MSTime timeNow) :
            executor(Executor),
            callback(0),
            objectId(0),
            now(timeNow)
        {
        }

        ITaskExecutor& executor;
        ICallBack* callback;
        TaskTarget* objectId;
        const MSTime now;
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
        bool isRegisteredIn(const ITaskExecutor * _owner) const { return isRegistered() && _owner == m_executor;}
    public:
        bool isRegistered() const { return m_objectId.isRegistered();}
        bool hasExecutor() const { return m_executor != 0;}
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
