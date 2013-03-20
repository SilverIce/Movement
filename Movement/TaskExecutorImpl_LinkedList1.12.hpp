namespace Tasks { namespace detail
{
    class TaskExecutorImpl_LinkedList112
    {
    public:

        struct Node;
        struct NullValue{};
        struct TimeComparator;
        struct ObjectIdComparator;

        typedef TaskTargetImpl::element_type TaskTargetNode;

        typedef Movement::LinkedList<NullValue> SortedTaskList;

        struct MarkInfo {
            uint32 time;
            Node* node;
            bool operator == (const MarkInfo& right) const {
                return node == right.node;
            }
        };
        typedef SortedList<MarkInfo, TimeComparator> MarkArray;

        struct Node : public SortedTaskList::element_type, public TaskHandle
        {
            uint32 execution_time;
            CallBack* callback;
            TaskTarget* taskTarget;     // can be null
            TaskTargetNode tasknode;

            Node() {
                execution_time = 0;
                callback = 0;
                taskTarget = 0;
                tasknode.Value = this;
            }

            ~Node() { clear();}

            void clear()
            {
                delink();
                if (taskTarget)
                    TaskTargetImpl::cast(*taskTarget).delink(tasknode);
                execution_time = 0;
                callback = 0;
                taskTarget = 0;
            }

            bool cleaned() const {
                return !callback && !tasknode.linked() && !linked() && !taskTarget;
            }

            bool isMarkNode() const {
                return callback == nullptr;
            }
        };

        enum Config{
            roundConst = 128,
        };

        struct TimeComparator {
            bool operator()(const Node* left, const Node* right) {
                return left->execution_time > right->execution_time;
            }
            bool operator()(const MarkInfo& left, const MarkInfo& right) {
                return left.time > right.time;
            }
        };

        SortedTaskList top;
        Recycler3<Node> unusedNodes;
        MarkArray marks;

        // For debugging purposes

        // infinite Update-> AddTask-> Update detection:

        struct DBGFields {
            MSTime TimeNow;
            bool UpdateLoop;

            DBGFields() {
                UpdateLoop = false;
            }
        } debug;

        //////////////////////////////////////////////////////////////////////////

        ~TaskExecutorImpl_LinkedList112() { CancelAllTasks();}

        bool hasCallbacks() const {
            return top.size() > (uint32)marks.size();
        }

        void printStats()
        {
            log_console("tasks active: %u", top.size());
            if (!top.empty())
                log_console("delta time: %u ms", ((Node*)top.last())->execution_time - ((Node*)top.first())->execution_time);
            log_console("marks amount: %u", marks.size());
        }

        /*
            <-- the oldest tasks&marks                the youngest tasks&marks  -->
            O~   o    o     o     o     O~   o    o   o     O~  o   o   o

            "Legend":
            O~  node-mark, that helps insert tasks in fast way
            o   node-task 
        */

        void PushIntoList(Node * newNode)
        {
            MarkInfo mark = {(newNode->execution_time / roundConst + 1) * roundConst, 0};
            MarkArray::iterator it = marks.lower_bound(mark);
            if (it != marks.end() && it->time == mark.time)
                mark.node = it->node; // mark found
            else {
                // create mark node
                mark.node = unusedNodes.pop();
                mark.node->execution_time = mark.time;

                if (it == marks.end())
                    top.link_first(*mark.node);
                else {
                    InsertAfter(it->node, mark.node);
                }
                it = marks.insert(it, mark);
                ensureSorted();
            }

            const MarkInfo* rigthMark = (it+1 < marks.end() ? it : nullptr);
            if (rigthMark && (mark.time - newNode->execution_time) > (newNode->execution_time - rigthMark->time))
                InsertAfter(rigthMark->node, newNode); // never happens??? omg
            else
                InsertBefore(mark.node, newNode);

            ensureSorted();
        }

        /** Inserts a newNode in such way, that newNode.execution_time > first.execution_time
        */
        void InsertAfter(Node * first, Node * newNode) 
        {
            Node * node = first;
            uint32 time = newNode->execution_time;
            assert_state( time >= node->execution_time );
            while (true) {
                Node * next = (Node*)node->Next();
                if (!next || next->execution_time > time)
                    break;
                node = next;
            }
            assert_state( time >= node->execution_time );

            top.link_after(node, newNode);
        }

        /** Inserts a newNode in such way, that newNode.execution_time > first.execution_time
        */
        void InsertBefore(Node * first, Node * newNode) 
        {
            Node * node = first;
            uint32 time = newNode->execution_time;
            assert_state( node->execution_time > time );
            while (true) {
                Node * next = (Node*)node->Previous();
                if (!next || time >= next->execution_time)
                    break;
                node = next;
            }
            assert_state( node->execution_time > time );

            top.link_before(node, newNode);
        }

        void AddTask(CallBack* obj, MSTime exec_time, TaskTarget* target)
        {
            //assert_state(!debug.UpdateLoop || debug.TimeNow < exec_time);

            obj->addref();

            Node * newNode = unusedNodes.pop();
            //assert_state( newNode->cleaned() );
            newNode->callback = obj;
            newNode->execution_time = exec_time.time;
            //newNode->objectId = target.objectId;
            newNode->taskTarget = target;

            if (target)
                TaskTargetImpl::cast(*target).link_last(newNode->tasknode);
            PushIntoList(newNode);
            ensureSorted();
        }

        void CancelTasks(TaskTarget& target)
        {
            TaskTargetImpl& list = TaskTargetImpl::cast(target);
            if (list.empty())
                return;

            while (TaskTargetNode * targetNode = list.first()) {
                Node * node = static_cast<Node*>(targetNode->Value);
                node->callback->release();
                unusedNodes.push(node);
            }
            assert_state( list.empty() );
            ensureSorted();
        }

        void CancelAllTasks()
        {
            while (Node * firstNode = static_cast<Node*>(top.first()))
            {
                if (!firstNode->isMarkNode())
                    firstNode->callback->release();
                else {
                    assert_state(firstNode == marks.back().node);
                    marks.pop_back();
                }
                unusedNodes.push(firstNode);
            }
            assert_state(top.empty());
            assert_state(marks.empty());
        }

        void ensureSorted()
        {
            /*if (top.empty())
                return;

            Node * node = (Node*)top.first();
            uint32 time = node->execution_time;
            int cycle = 0;
            std::vector<Node*> nodes(1,node);

            while (node = (Node*)node->Next()) {
                nodes.push_back(node);
                assert_state( node->execution_time >= time);
                time = node->execution_time;
                ++cycle;
            }*/
        }

        void Execute(TaskExecutor_Args& args)
        {
            ensureSorted();

            debug.TimeNow = args.now;
            debug.UpdateLoop = true;

            Node * firstNode = NULL;
            while ((firstNode = static_cast<Node*>(top.first())) && firstNode->execution_time <= args.now.time)
            {
                if (!firstNode->isMarkNode())
                {
                    args.callback = firstNode->callback;
                    args.objectId = firstNode->taskTarget;

                    args.callback->Execute(args);
                    args.callback->release();
                }
                else
                {
                    // It's possible that during task execution some new mark will be appended to mark array end?
                    assert_state(firstNode == marks.back().node);
                    marks.pop_back();
                }
                unusedNodes.push(firstNode);
            }

            debug.UpdateLoop = false;
        }
    };
}
}
