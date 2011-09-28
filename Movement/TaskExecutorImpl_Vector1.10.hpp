namespace Tasks
{
    class TaskExecutorImpl_Vector110
    {
    public:
        struct TaskInternal 
        {
            uint32 execution_time;
            ObjectId objectId;
            CallBack* task;
        };

        struct TaskComparator {
            bool operator()(const TaskInternal& left, const TaskInternal& right) {
                return left.execution_time > right.execution_time;
            }
        };

        typedef std::vector<TaskInternal> TaskContainer;
        TaskContainer tasks;
        TaskContainer copy_container;
        //std::vector<ObjectId> objects;
        ObjectCounter m_counter;


        void AddTask(CallBack* obj, MSTime exec_time, ObjectId objectId)
        {
            TaskInternal task = {exec_time.time, objectId, obj};
            // inserts values in order:
            // 30, 20, 10, .. end
            // so, most active elements are in end of container - this leads erase and insert operations affect
            // container's end. In most cases it is efficient for std::vector

            tasks.insert(std::lower_bound(tasks.begin(),tasks.end(), task, TaskComparator()), task);
            obj->addref();
        }

        void RegisterObject(ObjectId& obj)
        {
            obj = m_counter.NewId();
        }

        void CancelTasks(ObjectId objectId)
        {
            struct {
                ObjectId objectId;
                inline bool operator()(TaskInternal& t){
                    if (t.objectId == objectId){
                        t.task->release();
                        return true;
                    }
                    return false;
                }
            } task_remover = {objectId};
            tasks.erase(std::remove_if(tasks.begin(),tasks.end(),task_remover), tasks.end());
        }

        ~TaskExecutorImpl_Vector110() { CancelAllTasks();}

        void CancelAllTasks()
        {
            struct task_remover {
                inline void operator()(TaskInternal& t) { t.task->release();}
            };

            TaskContainer copy;
            copy.swap(tasks);
            std::for_each(copy.begin(), copy.end(), task_remover());    
        }

        void RemoveObject(ObjectId& obj)
        {
            CancelTasks(obj);
        }

        void Update(TaskExecutor_Args& args)
        {
            TaskInternal fakeTask = {args.now.time, 0, 0};
            TaskContainer::iterator tasksEnd(tasks.end());
            TaskContainer::iterator it(
                std::lower_bound(tasks.begin(),tasksEnd, fakeTask, TaskComparator())
            );

            if (it == tasksEnd)
                return;

            copy_container.assign(it, tasksEnd);
            tasks.erase(it, tasksEnd);

            // Need execute in proper(reverse) order. task_processor may lead to deep and unsafe calls
            struct {
                TaskExecutor_Args& _args;
                inline void operator()(TaskInternal& t)
                {
                    _args.objectId.objectId = t.objectId;
                    _args.callback = t.task;
                    t.task->execute(_args);  // this might be unsafe & deep call
                    t.task->release();
                }
            } task_processor = {args};
            std::for_each(copy_container.rbegin(), copy_container.rend(), task_processor);    
        }
    };
}
