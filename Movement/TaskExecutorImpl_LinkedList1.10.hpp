namespace Tasks
{
    class ObjectId_hash_compare
    {
        typedef std::less<ObjectId> _Pr;
    public:
        enum {    // parameters for hash table
            bucket_size = 4,
            min_buckets = 8    // min_buckets = 2 ^^ N, 0 < N
        };

        ObjectId_hash_compare() : comp()
        {}

        ObjectId_hash_compare(_Pr _Pred) : comp(_Pred)
        {}

        size_t operator()(const ObjectId& _Keyval) const {
            return _Keyval;
        }

        bool operator()(const ObjectId& _Keyval1, const ObjectId& _Keyval2) const {
            return (comp(_Keyval1, _Keyval2));
        }

    protected:
        _Pr comp;    // the comparator object
    };

#define SELECTED_CONT std::vector
// #define SELECTED_CONT POD_Array

    template<class T>
    class Recycler
    {
    protected:
        SELECTED_CONT<T*> _data;
        uint32 _allocated;
    public:

        Recycler() {
            clear();
        }

        ~Recycler() {
            assert_state(_allocated == 0);
            clear();
        }

        void push(T* obj) {
            assert_state(_allocated > 0);
            --_allocated;
            obj->clear();
            _data.push_back(obj);
        }

        T* pop() {
            ++_allocated;
            if (_data.empty())
                return new T();
            else {
                T * obj = _data.back();
                _data.pop_back();
                return obj;
            }
        }

        bool cleaned() const {
            return _allocated == 0;
        }

        void clear() {
            ForEach(T* obj, _data, delete obj);
            _data.clear();
            _allocated = 0;
        }
    };

    template<class T, class Pred = std::less<T> >
    class SortedList : public SELECTED_CONT<T>
    {
        typedef SELECTED_CONT<T> base;
    public:

        iterator lower_bound(const T& value) {
            return std::lower_bound(begin(),end(),value,Pred());
        }

        void insert_unique(T& value) {
            iterator it = lower_bound(value);
            if (it == end() || *it != value)
                base::insert(it, value);
        }

        void insert(T& value) {
            insert(lower_bound(value), value);
        }

        using base::insert;
        using base::erase;

        void erase(const T& value) {
            iterator it(lower_bound(value));
            if (it != end() && *it == value)
                base::erase(it);
        }
    };

    template<typename T>
    class RecyclerPull
    {
        POD_Array<uint32> _free;
        uint32 NodesActive;
        const uint32 PullSize;
        T* nodes;
        POD_Array<T*> _dynAllocated;

        bool isDymamicallyAllocated(const T* obj) const {
            return (obj < nodes) || (obj > (nodes+PullSize-1));
        }

        void init() {
            NodesActive = 0;
            _free.clear();
            _free.reserve(PullSize);
            uint32 i = PullSize;
            while(i--)
                _free.push_back(i);
        }
    public:

        uint32 PullTotalSize() const { return PullSize;}
        uint32 PullAllocActiveSize() const { return PullSize - _free.size();}

        uint32 TotalActiveSize() const { return NodesActive;}
        uint32 DynamicActiveSize() const { return NodesActive - PullAllocActiveSize();}

    public:
        RecyclerPull(uint32 pullSize = 800) : PullSize(pullSize) {
            nodes = new T[pullSize];
            init();
        }

        ~RecyclerPull() {
            clear();
            delete[] nodes;
            nodes = NULL;
        }

        T* pop() {
            ++NodesActive;
            if (_free.empty())
                return new T();
            else {
                T* obj = nodes + _free.back();
                _free.pop_back();
                return obj;
            }
        }

        void push(T* node) {
            node->clear();
            if (!isDymamicallyAllocated(node))
                _free.push_back(node - nodes);
            else
                delete node;
            assert_state(NodesActive > 0);
            --NodesActive;
        }

        bool cleaned() const {
            return NodesActive == 0;
        }

        void clear() {
            assert_state( cleaned() );
            init();
        }
    };

    class TaskExecutorImpl_LinkedList110
    {
    public:

        struct Node;
        struct NullValue{};
        struct TimeComparator;
        struct ObjectIdComparator;

        typedef Movement::LinkedList<Node*> TaskList;
        typedef TaskList::element_type TaskNode;

        typedef stdext::hash_map<ObjectId, TaskList* /*,ObjectId_hash_compare*/ > NodeTable;
        struct NodeTableEntry {
            ObjectId objectId;
            TaskList* list;
        };
        /*typedef SortedList<NodeTableEntry, ObjectIdComparator> NodeTable;*/

        typedef Movement::LinkedList<NullValue> LinkedList;

        struct MarkInfo {
            uint32 time;
            Node* node;
            bool operator == (const MarkInfo& right) const {
                return node == right.node;
            }
        };
        typedef SortedList<MarkInfo, TimeComparator> MarkArray;

        struct Node : public LinkedList::element_type
        {
            uint32 execution_time;
            ObjectId objectId;
            CallBack* callback;
            TaskNode tasknode;

            Node() {
                execution_time = 0;
                objectId = 0;
                callback = 0;
                tasknode.Value = this;
            }

            ~Node() { clear();}

            void clear()
            {
                if (linked())
                    List().delink(*this);
                execution_time = 0;
                objectId = 0;
                callback = 0;
                if (tasknode.linked())
                    tasknode.List().delink(tasknode);
            }

            bool cleaned() const {
                return !objectId && !callback && !tasknode.linked() && !linked();
            }

            bool isMarkNode() {
                return callback == 0;
            }

            bool empty() {
                return callback == 0;
            }
        };

        enum Config{
            roundConst = 128,
            NodePullSize = 5000,
            NodeListPullSize = 500,
        };

        struct TimeComparator {
            bool operator()(const Node* left, const Node* right) {
                return left->execution_time > right->execution_time;
            }
            bool operator()(const MarkInfo& left, const MarkInfo& right) {
                return left.time > right.time;
            }
        };

        struct ObjectIdComparator {
            bool operator()(const NodeTableEntry& left, const NodeTableEntry& right) {
                return left.objectId < right.objectId;
            }
        };

        LinkedList top;
        NodeTable callbacks;
        RecyclerPull<TaskList> unusedTaskLists;
        RecyclerPull<Node> unusedNodes;
        MarkArray marks;
        POD_Array<CallBack*> callbackTempList;

        TaskExecutorImpl_LinkedList110() :
            unusedNodes(NodePullSize),
            unusedTaskLists(NodeListPullSize)
        {
        }

        ~TaskExecutorImpl_LinkedList110() { CancelAllTasks();}

        bool hasCallbacks() const {
            return !callbacks.empty();
        }

        void printStats()
        {
            log_console("tasks active: %u", top.size());
            log_console("delta time: %u ms", ((Node*)top.last())->execution_time - ((Node*)top.first())->execution_time);
            log_console("marks amount: %u", marks.size());

            #define _PULL_STAT_(pull, stat) log_console(#stat " %u", pull.stat());
            #define PULL_STAT(pull) \
                log_console(#pull " stats"); \
                _PULL_STAT_(pull, PullTotalSize); \
                _PULL_STAT_(pull, PullAllocActiveSize); \
                _PULL_STAT_(pull, TotalActiveSize); \
                _PULL_STAT_(pull, DynamicActiveSize);

            PULL_STAT(unusedNodes);
            PULL_STAT(unusedTaskLists);
            #undef PULL_STAT
            #undef _PULL_STAT_
        }


        /** Returns a such node, so node's execution_time > time
            to not break a sorted nodes sequence need insert after node */
        Node* GetPlaceAfter(Node * node, uint32 time)
        {
            assert_state( time >= node->execution_time );
            while (true) {
                Node * next = (Node*)node->Next();
                if (!next || next->execution_time > time)
                    break;
                node = next;
            }
            assert_state( time >= node->execution_time );
            return node;
        }

        /** Returns a such node, so node's execution_time > time
            to not break a sorted nodes sequence need insert before node */
        Node* GetPlaceBefore(Node * node, uint32 time)
        {
            assert_state( node->execution_time > time );
            while (true) {
                Node * next = (Node*)node->Previous();
                if (!next || time >= next->execution_time)
                    break;
                node = next;
            }
            assert_state( node->execution_time > time );
            return node;
        }

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
                    Node * node = GetPlaceAfter(it->node, mark.time);
                    top.link_after(node, mark.node);
                }
                marks.insert(it, mark);
                ensureSorted();
            }

            Node * node = GetPlaceBefore(mark.node, newNode->execution_time);
            top.link_before(node, newNode);
            ensureSorted();
        }

        void AddTask(CallBack* obj, MSTime exec_time, ObjectId objectId)
        {
            obj->addref();

            Node * newNode = unusedNodes.pop();
            //assert_state( newNode->cleaned() );
            newNode->callback = obj;
            newNode->execution_time = exec_time.time;
            newNode->objectId = objectId;

            PushIntoList(newNode);
            PushIntoTable(newNode);
            ensureSorted();
        }

        void PushIntoTable(Node * newNode)
        {
            TaskList * list = NULL;
            NodeTable::iterator it = callbacks.find(newNode->objectId);
            if (it != callbacks.end())
                list = it->second;
            else {
                list = unusedTaskLists.pop();
                callbacks.insert(NodeTable::value_type(newNode->objectId, list));
            }
            list->link_last(newNode->tasknode);
        }

        void RegisterObject(ObjectId& obj)
        {
        }

        void CancelTasks(ObjectId objectId)
        {
            NodeTable::iterator it = callbacks.find(objectId);
            if (it == callbacks.end())
                return;

            TaskList& list = *it->second;
            callbacks.erase(it);

            callbackTempList.clear();
            while (TaskNode * node = list.last()) {
                callbackTempList.push_back(node->Value->callback);
                unusedNodes.push(node->Value);
            }

            assert_state( list.empty() );
            unusedTaskLists.push(&list);
            ensureSorted();

            ForEach(CallBack* cb, callbackTempList, cb->release());
        }

        void CancelAllTasks()
        {
            while(!callbacks.empty())
                CancelTasks(callbacks.begin()->first);

            // 'top' still contains mark-nodes
            while (Node * node = (Node*)top.last()) {
                unusedNodes.push(node);
            }
            marks.clear();
        }

        void RemoveObject(ObjectId& obj)
        {
            CancelTasks(obj);
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

        void Update(TaskExecutor_Args& args)
        {
            ensureSorted();
            Node * firstNode = NULL;
            while ((firstNode = (Node*)top.first()) && firstNode->execution_time <= args.now.time)
            {
                CallBack * callback = NULL;
                if (!firstNode->isMarkNode())
                {
                    TaskList& list = firstNode->tasknode.List();
                    if (list.size() == 1) {
                        callbacks.erase(firstNode->objectId);
                        unusedTaskLists.push(&list);
                    }
                    args.callback = callback = firstNode->callback;
                    args.objectId.objectId = firstNode->objectId;
                }
                else
                {
                    MarkInfo info = {firstNode->execution_time, firstNode};
                    marks.erase(info);
                }
                unusedNodes.push(firstNode);

                if (callback) {
                    callback->Execute(args);
                    callback->release();
                }
            }
        }
    };
}
