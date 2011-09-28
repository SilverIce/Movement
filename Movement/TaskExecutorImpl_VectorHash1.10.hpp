namespace Tasks
{
    class TaskExecutorImpl_VectorHashSimple110
    {
    public:
        struct TaskInternal 
        {
            uint32 execution_time;
            ObjectId objectId;
            CallBack* callback;
        };

        struct TimeComparator {
            bool operator()(const TaskInternal& left, const TaskInternal& right) {
                return left.execution_time > right.execution_time;
            }
        };

        typedef std::vector<TaskInternal> TaskContainer;
        typedef stdext::hash_map<ObjectId, bool /*canceled*/> OwnerSet;

        TaskContainer copy_container;
        TaskContainer tasks;
        ObjectCounter m_counter;
        OwnerSet m_owners;

        void AddTask(CallBack* obj, MSTime exec_time, ObjectId objectId)
        {
            TaskInternal task = {exec_time.time, objectId, obj};
            // inserts values in order:
            // 30, 20, 10, .. end
            // so, most active elements are in end of container - this leads erase and insert operations affect
            // container's end. In most cases it is efficient for std::vector
            tasks.insert(std::lower_bound(tasks.begin(),tasks.end(), task, TimeComparator()), task);
            obj->addref();
        }

        void RegisterObject(ObjectId& obj)
        {
            obj = m_counter.NewId();
            m_owners.insert(OwnerSet::value_type(obj, false));
        }

        void RemoveObject(ObjectId& obj)
        {
            CancelTasks(obj);
            m_owners.erase(obj);
        }

        void CancelTasks(ObjectId objectId)
        {
            OwnerSet::iterator it = m_owners.find(objectId);
            if (it != m_owners.end())
                it->second = true;
        }

        ~TaskExecutorImpl_VectorHashSimple110() { CancelAllTasks();}

        void CancelAllTasks()
        {
            struct task_remover {
                void operator()(TaskInternal& t) { t.callback->release();}
            };
            std::for_each(tasks.begin(), tasks.end(), task_remover());    
            tasks.clear();
        }

        /*void insertPending()
        {
            if (!m_insert_pending.empty())
            {
                std::sort(m_insert_pending.begin(), m_insert_pending.end(), TimeComparator());
                tasks.reserve(tasks.size() + m_insert_pending.size());

                struct {
                    TaskContainer& tasks;
                    size_t timesIt;
                    void operator()(const TimeInternal& t)
                    {
                        TaskContainer::iterator it(
                            std::lower_bound(tasks.begin()+timesIt, tasks.end(), t, TimeComparator()));
                        timesIt = it - tasks.begin();
                        tasks.insert(it, t);
                    }
                } inserter = {tasks, 0};
                std::for_each(m_insert_pending.begin(), m_insert_pending.end(), inserter);
                m_insert_pending.clear();
            }
        }*/

        void Update(TaskExecutor_Args& args)
        {
            //insertPending();

            TaskInternal fake = {args.now.time, 0, NULL};
            TaskContainer::iterator timesEnd(tasks.end());
            TaskContainer::iterator it(
                std::lower_bound(tasks.begin(),timesEnd, fake, TimeComparator())
            );

            if (it == timesEnd)
                return;

            copy_container.assign(it, timesEnd);
            tasks.erase(it, timesEnd);

            // Need execute in proper(reverse) order. task_processor may lead to deep and unsafe calls
            struct {
                inline void operator()(TaskInternal& task)
                {
                    OwnerSet::const_iterator it = owners.find(task.objectId);
                    if (it == owners.end() || it->second)
                        return;
                    _args.objectId.objectId = task.objectId;
                    _args.callback = task.callback;
                    task.callback->execute(_args);  // this might be unsafe & deep call
                    task.callback->release();
                }
                TaskExecutor_Args _args;
                OwnerSet& owners;
            } task_processor = {args, m_owners};
            std::for_each(copy_container.rbegin(), copy_container.rend(), task_processor);    
        }
    };
}
