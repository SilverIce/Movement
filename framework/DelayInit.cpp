#include "DelayInit.h"
#include "typedefs_p.h"

namespace delayInit
{
    void register_node(node * n);

    node::node(CTor creator) {
        this->next = 0;
        this->ctor = creator;
        register_node(this);
    }

    static node * first = 0;
    static node * last = 0;
    static unsigned int listSize = 0;
    static bool delayInit_called = false;

    void register_node(node * n) {
        if (!first)
            first = n;
        if (last)
            last->next = n;
        last = n;
        ++listSize;
    }

    void callCtors() {
        // function assumes that it's safe to assert before delayed object initialization
        assert_state(delayInit_called == false);
        delayInit_called = true;

        node * n = first;
        while(n) {
            n->ctor();
            n = n->next;
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
