#pragma once

#include "typedefs.h"
#include "MSTime.h"

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
        virtual void CancelTasks(const TaskTarget& ownerId) = 0;
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
        void CancelTasks(const TaskTarget& ownerId) override;
        void CancelAllTasks();

        void Register(TaskTarget& obj) override;
        void Unregister(TaskTarget& obj) override;

        void Update(MSTime time) override;
    };

    typedef uint32 ObjectId;

    class TaskTarget
    {
    private:
        friend class TaskExecutor;
        template<class T> friend class taskExecutor;// temp
    public:     // temp
        ObjectId objectId;
    private:
        //NON_COPYABLE(TaskTarget);
        explicit TaskTarget(ObjectId Id) : objectId(Id) {}
    public:
        bool isRegistered() const { return objectId != 0;}
        explicit TaskTarget() : objectId(0) {}
        ~TaskTarget();
    };

    struct TaskExecutor_Args
    {
        ITaskExecutor& executor;
        ICallBack* callback;
        const MSTime now;
        TaskTarget objectId;
    };

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
}
