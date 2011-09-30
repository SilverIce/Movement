#pragma once

#include "typedefs.h"
#include "MSTime.h"

namespace Tasks
{
    using namespace BasicTypes;
    using Movement::MSTime;

#define NON_COPYABLE(Class) \
    Class(const Class &); \
    Class& operator = (const Class &);

#define NON_COPYABLE_NO_DEFAULT(Class) \
    NON_COPYABLE(Class) \
    explicit Class(); \

    struct TaskExecutor_Args;
    class TaskExecutor;
    class CallBackPublic;
    class ITaskExecutor;
    class TaskTarget;
    class CallBackPublic;

    class ITaskExecutor
    {
    public:
        virtual void AddTask(CallBackPublic * callback, MSTime exec_time, const TaskTarget& ownerId) {}
        virtual void CancelTasks(const TaskTarget& ownerId) {}
        virtual void Update(MSTime time) {}
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
        void AddTask(T * functor, MSTime exec_time, const TaskTarget& ownerId);
        void AddTask(CallBackPublic * callback, MSTime exec_time, const TaskTarget& ownerId) override;

        void CancelTasks(const TaskTarget& ownerId) override;
        void CancelAllTasks();

        void RegisterObject(TaskTarget& obj);
        void RemoveObject(TaskTarget& obj);

        void Update(MSTime time) override;

        bool HasCallBacks() const;
    };

    typedef void (*ExecFunc)(void*, TaskExecutor_Args*);

    class CallBackPublic
    {
    private:
        NON_COPYABLE(CallBackPublic);
        static CallBackPublic* create(void* functor, ExecFunc execFunc);
    protected:
        CallBackPublic() {}
        ~CallBackPublic() {}
    public:
        /** creates callback object. functor object should be allocated dynamically
            since his lifetime binded to callback's lifetime */
        template<class T>
        static CallBackPublic* create(T* functor) {
            return create(functor, &T::Static_Execute);
        }
    };

    typedef uint32 ObjectId;

    class TaskTarget 
    {
    private:
        friend class TaskExecutor;
        template<class T> friend class taskExecutor;// temp
        friend class TaskExecutorImpl_Vector110;// temp
    public:
        ITaskExecutor * owner;
        ObjectId objectId;
    private:
        //NON_COPYABLE(TaskTarget);
        explicit TaskTarget(ITaskExecutor * own, ObjectId Id) : owner(own), objectId(Id) {}
    public:
        bool isRegisteredIn(const ITaskExecutor * _owner) const { return _owner == owner;}
        bool isRegistered() const { return owner != 0;}
        explicit TaskTarget() : owner(0), objectId(0) {}
    };

#define STATIC_EXEC(Class, Args) \
    static void Static_Execute(void* _me, TaskExecutor_Args* _args){ \
        if (_args) \
            ((Class*)_me)->Class::Member_Execute(*_args); \
        else \
            delete ((Class*)_me); \
    } \
    private: \
    inline void Member_Execute(Args)

    struct TaskExecutor_Args
    {
        ITaskExecutor& executor;
        CallBackPublic* callback;
        const MSTime now;
        TaskTarget objectId;
    };

    template<class T>
    inline void TaskExecutor::AddTask(T * functor, MSTime exec_time, const TaskTarget& ownerId)
    {
        AddTask(CallBackPublic::create(functor), exec_time, ownerId);
    }
}
