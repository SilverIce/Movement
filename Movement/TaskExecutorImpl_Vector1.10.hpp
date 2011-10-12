namespace Tasks
{
    class TaskExecutorImpl_VectorPOD110
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

        typedef POD_Array<TaskInternal> TaskContainer;
        TaskContainer tasks;
        TaskContainer copy_container;
        //std::vector<ObjectId> objects;


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

        ~TaskExecutorImpl_VectorPOD110() { CancelAllTasks();}

        void CancelAllTasks()
        {
            struct task_remover {
                inline void operator()(TaskInternal& t) { t.task->release();}
            };

            std::for_each(tasks.begin(), tasks.end(), task_remover());
            tasks.clear();
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
            tasks.resize(tasks.size() - (tasksEnd-it));

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
