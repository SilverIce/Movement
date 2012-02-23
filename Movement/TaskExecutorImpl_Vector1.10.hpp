namespace Tasks
{
/*
    class TaskExecutorImpl_VectorPOD110
    {
    public:

        void printStats() {}

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

        typedef std::vector<TaskInternal> TaskList;
        TaskList tasks;

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

        bool hasCallbacks() const
        {
            return !tasks.empty();
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
            TaskList::reverse_iterator it;
            while ((it=tasks.rbegin()) != tasks.rend() && it->execution_time <= args.now.time)
            {
                TaskInternal& _internal = *it;
                CallBack* task = _internal.task;
                args.objectId.objectId = _internal.objectId;
                args.callback = task;
                tasks.pop_back();

                task->Execute(args);  // might be unsafe & deep call
                task->release();
            }
        }
    };
*/
}
