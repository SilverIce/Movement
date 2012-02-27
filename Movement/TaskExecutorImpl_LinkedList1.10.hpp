namespace Tasks
{
//#define SELECTED_CONT std::vector
 #define SELECTED_CONT POD_Array

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

    /** A simple linkedlist based recycler. 
        TODO: how to ensure that T is LinkedListElement<V> ?
    */
    template<class T>
    class Recycler3
    {
    protected:
        Movement::LinkedList<typename T::value_type> _data;
        uint32 _allocated;
    public:

        Recycler3() {
            clear();
        }

        ~Recycler3() {
            assert_state(_allocated == 0);
            clear();
        }

        void push(T* obj) {
            assert_state(_allocated > 0);
            --_allocated;
            obj->clear();
            _data.link_last(*obj);
        }

        T* pop() {
            ++_allocated;
            if (_data.empty())
                return new T();
            else {
                T * obj = static_cast<T*>(_data.last());
                _data.delink_last();
                return obj;
            }
        }

        bool cleaned() const {
            return _allocated == 0;
        }

        void clear() {
            while( T * node = static_cast<T*>(_data.last()) ) {
                _data.delink_last();
                delete node;
            }
            assert_state(_data.empty());
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

    template<typename T>
    class RecyclerPullFake
    {
        uint32 NodesActive;
    public:

        uint32 PullTotalSize() const { return 0;}
        uint32 PullAllocActiveSize() const { return 0;}

        uint32 TotalActiveSize() const { return NodesActive;}
        uint32 DynamicActiveSize() const { return NodesActive - PullAllocActiveSize();}

    public:
        RecyclerPullFake(uint32 pullSize = 800) : NodesActive(0) {
        }

        ~RecyclerPullFake() {
            clear();
        }

        T* pop() {
            ++NodesActive;
            return new T;
        }

        void push(T* node) {
            delete node;
            assert_state(NodesActive > 0);
            --NodesActive;
        }

        bool cleaned() const {
            return NodesActive == 0;
        }

        void clear() {
            assert_state( cleaned() );
        }
    };

    class TaskExecutorImpl_LinkedList110
    {
    public:

        struct Node;
        struct NullValue{};
        struct TimeComparator;
        struct ObjectIdComparator;

        typedef Movement::LinkedList<TaskHandle*> TaskTargetList;
        typedef TaskTargetList::element_type TaskTargetNode;

        //typedef stdext::hash_map<ObjectId, TaskList* /*,ObjectId_hash_compare*/ > NodeTable;
        /*struct NodeTableEntry {
            ObjectId objectId;
            TaskList* list;
        };
        typedef SortedList<NodeTableEntry, ObjectIdComparator> NodeTable;*/

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
            TaskTarget* taskTarget; 
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
                if (linked())
                    List().delink(*this);
                execution_time = 0;
                callback = 0;
                taskTarget = 0;
                if (tasknode.linked())
                    tasknode.List().delink(tasknode);
            }

            bool cleaned() const {
                return !callback && !tasknode.linked() && !linked() && !taskTarget;
            }

            bool isMarkNode() {
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

        SortedTaskList top;
        Recycler3<Node> unusedNodes;
        MarkArray marks;

        TaskExecutorImpl_LinkedList110()
        {
        }

        ~TaskExecutorImpl_LinkedList110() { CancelAllTasks();}

        bool hasCallbacks() const {
            return top.size() > marks.size();
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

            //PULL_STAT(unusedNodes);
            //PULL_STAT(unusedTaskLists);
            #undef PULL_STAT
            #undef _PULL_STAT_
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
                    InsertAfter(it->node, mark.node);
                }
                marks.insert(it, mark);
                ensureSorted();
            }

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

        void AddTask(CallBack* obj, MSTime exec_time, TaskTarget& target)
        {
            obj->addref();

            Node * newNode = unusedNodes.pop();
            //assert_state( newNode->cleaned() );
            newNode->callback = obj;
            newNode->execution_time = exec_time.time;
            //newNode->objectId = target.objectId;
            newNode->taskTarget = &target;

            PushIntoList(newNode);
            PushIntoTable(newNode, target);
            ensureSorted();
        }

        void PushIntoTable(Node * newNode, TaskTarget& target)
        {
            getImpl(target).list.link_last(newNode->tasknode);
        }

        void CancelTasks(TaskTarget& target)
        {
            TaskTargetList& list = getImpl(target).list;
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
                    MarkInfo info = {firstNode->execution_time, firstNode};
                    marks.erase(info);
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

        void Update(TaskExecutor_Args& args)
        {
            ensureSorted();
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
                    MarkInfo info = {firstNode->execution_time, firstNode};
                    marks.erase(info);
                }
                unusedNodes.push(firstNode);
            }
        }
    };
}
