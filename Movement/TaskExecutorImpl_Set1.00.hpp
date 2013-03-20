#include <set>
//#include "rb_tree.h"

namespace Tasks { namespace detail
{
    class TaskExecutorImpl_Set100
    {
    public:

        struct Node;
        struct NullValue{};
        struct TimeComparator;
        struct ObjectIdComparator;

        typedef TaskTargetImpl::element_type TaskTargetNode;

        struct TimeComparator {
            bool operator()(const Node* left, const Node* right);
        };

        typedef std::multiset<Node*,struct TimeComparator> SortedTaskList;

        struct Node : /*public RedBlackEntry,*/ public TaskHandle
        {
            uint32 execution_time;
            CallBack* callback;
            TaskTarget* taskTarget;     // can be null
            TaskTargetNode tasknode;
            SortedTaskList::iterator self;

            Node() {
                execution_time = 0;
                callback = 0;
                taskTarget = 0;
                tasknode.Value = this;
            }

            ~Node() { assert_state(cleaned());}

            void clear() {}

            void clear(SortedTaskList& list)
            {
                list.erase(self);
                if (taskTarget)
                    TaskTargetImpl::cast(*taskTarget).delink(tasknode);
                execution_time = 0;
                callback = 0;
                taskTarget = 0;
            }

            bool cleaned() const {
                return !callback;
            }

            virtual int GetKey() const {
                return (int)execution_time;
            }
        };

        enum Config{
            roundConst = 128,
        };

        SortedTaskList top;
        RecyclerFake<Node> unusedNodes;

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

        TaskExecutorImpl_Set100()
        {
        }

        ~TaskExecutorImpl_Set100() { CancelAllTasks();}

        bool hasCallbacks() const {
            return !top.empty();
        }

        void printStats()
        {
            log_console("tasks active: %u", top.size());
            /*if (!top.empty())
                log_console("delta time: %u ms", ((Node*)top.last())->execution_time - ((Node*)top.first())->execution_time);
            log_console("marks amount: %u", marks.size());*/
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
            newNode->self = top.insert(newNode);
            ensureSorted();
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
                TaskTargetImpl::cast(*target).link_first(newNode->tasknode);
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
                node->clear(top);
                unusedNodes.push(node);
            }
            assert_state( list.empty() );
            ensureSorted();
        }

        void CancelAllTasks()
        {
            while (!top.empty())
            {
                Node * firstNode = *top.begin();
                firstNode->callback->release();
                firstNode->clear(top);
                unusedNodes.push(firstNode);
            }
            assert_state(top.empty());
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

            Node * firstNode = nullptr;
            while (!top.empty() && (firstNode = *top.begin(), firstNode->execution_time <= args.now.time))
            {
                args.callback = firstNode->callback;
                args.objectId = firstNode->taskTarget;

                args.callback->Execute(args);
                args.callback->release();
                firstNode->clear(top);
                unusedNodes.push(firstNode);
            }

            debug.UpdateLoop = false;
        }
    };

    inline bool TaskExecutorImpl_Set100::TimeComparator::operator ()(const Node* left, const Node* right) {
        return left->execution_time < right->execution_time;
    }

}
}
