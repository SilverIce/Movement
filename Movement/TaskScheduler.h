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
    class CallBack;
    class ITaskExecutor;
    class TaskTarget;

    class ITaskExecutor
    {
    public:
        virtual void AddTask(CallBack * callback, MSTime exec_time, TaskTarget& ownerId) = 0;
        virtual void CancelTasks(const TaskTarget& ownerId) = 0;
        virtual void Update(MSTime time) = 0;
        virtual void Register(TaskTarget& obj) = 0;
        virtual void Unregister(TaskTarget& obj) = 0;
    protected:
        ~ITaskExecutor() {}
    };

    class TaskExecutor : public ITaskExecutor
    {
        class TaskExecutorImpl& impl;
        int32 m_objectsRegistered;

        NON_COPYABLE(TaskExecutor);
    public:

        TaskExecutor();
        ~TaskExecutor();

        template<class T>
        void AddTask(T * functor, MSTime exec_time, TaskTarget& ownerId);
        void AddTask(CallBack * callback, MSTime exec_time, TaskTarget& ownerId) override;

        void CancelTasks(const TaskTarget& ownerId) override;
        void CancelAllTasks();

        void Register(TaskTarget& obj) override;
        void Unregister(TaskTarget& obj) override;

        void Update(MSTime time) override;

        bool HasCallBacks() const;
    };

    typedef void (*ExecFunc)(void*, TaskExecutor_Args*);

    CallBack* CallBackPublic(void* functor, ExecFunc execFunc);

    template<class T>
    inline CallBack* CallBackPublic(T* functor) {
        return CallBackPublic(functor, &T::Static_Execute);
    }

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
        CallBack* callback;
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

        template<class T>
        void AddTask(T * functor, MSTime exec_time) {
            AddTask(CallBackPublic(functor), exec_time);
        }
        void AddTask(CallBack * callback, MSTime exec_time);
    };

    /** Tools:
    */

#define STATIC_EXEC(Class, Args) \
    static void Static_Execute(void* _me, TaskExecutor_Args* _args){ \
        if (_args) \
            ((Class*)_me)->Class::Member_Execute(*_args); \
        else \
            delete ((Class*)_me); \
    } \
    private: \
    inline void Member_Execute(Args)

    /** Set belongs = true if you wanna automatically free resources at task cancelling */
    template<class T, class Der, bool belongs>
    struct StaticExecutor {
        static void Static_Execute(void* _me, TaskExecutor_Args* _args) {
            if (_args)
                Der::Execute( *(T*)_me, *_args );
            else if (belongs)
                delete ((T*)_me);
        }
        static void readd(TaskExecutor_Args& args, MSTime time) {
            args.executor.AddTask(args.callback,args.now+time,args.objectId);
        }
    };

    template<class T, bool belongs>
    struct Executor {
        static void Static_Execute(void* _me, TaskExecutor_Args* _args){
            if (_args)
                ((T*)_me)->Execute(*_args);
            else if (belongs)
                delete ((T*)_me);
        }
        static void readd(TaskExecutor_Args& args, MSTime time) {
            args.executor.AddTask(args.callback,args.now+time,args.objectId);
        }
    };

    template<class T>
    inline void TaskExecutor::AddTask(T * functor, MSTime exec_time, TaskTarget& ownerId)
    {
        AddTask(CallBackPublic(functor), exec_time, ownerId);
    }
}
