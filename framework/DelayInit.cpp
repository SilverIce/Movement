#include "DelayInit.h"
#include "typedefs_p.h"

namespace delayInit
{
    static void register_node(node * n);

    node::node(CTor creator) {
        this->next = nullptr;
        this->ctor = creator;
        register_node(this);
    }

    static node * first = nullptr;
    static node * last = nullptr;
    static bool delayInit_called = false;

    void register_node(node * n) {
        assert_state_msg(delayInit_called == false, "attemp construct and register node is not allowed");
        if (!first)
            first = n;
        if (last)
            last->next = n;
        last = n;
    }

    void callCtors() {
        // function assumes that it's safe to assert before delayed object initialization
        assert_state(delayInit_called == false);
        delayInit_called = true;

        node * n = first, *n2 = nullptr;
        while(n) {
            n->ctor();
            n2 = n;
            n = n->next;
            n2->next = nullptr; // disconnect node
        }
    }
}

/* Example:
void doSomething() {
    // user code
    // for ex. it can be used to register some new type
}
static node node_doSomething(&doSomething);

struct SomeObj
{

} obj;
void InitSomeObj() {
    // user code
    // for ex. it can be used to register some new type
}
static node node_doSomething(&InitSomeObj);*/
