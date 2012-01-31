#pragma once

namespace delayInit
{
    struct node
    {
        typedef void (*CTor)();
        CTor ctor;
        node * next;

        explicit node(CTor creator);
    };

    void callCtors();

#define DELAYED_INIT2(Type, name, ...) \
    void FuncCtor_##name() { \
        name = new Type(__VA_ARGS__); \
        struct DTor { ~DTor() { delete name;} }; \
        static DTor _dtor; \
    } \
    DELAYED_CALL(FuncCtor_##name);

#define DELAYED_INIT(Type, name, ...) \
    void FuncCtor_##name() { \
        static Type * s_instance = 0; \
        s_instance = new Type(__VA_ARGS__); \
        struct DTor { ~DTor() { delete s_instance;} }; \
        static DTor _dtor; \
    } \
    DELAYED_CALL(FuncCtor_##name);

#define DELAYED_CALL_ARGS(Function, shortname, ...) \
    void FuncCaller_##shortname() { \
        Function(__VA_ARGS__); \
    } \
    static const ::delayInit::node caller_##shortname(&FuncCaller_##shortname);

#define DELAYED_CALL(Function) \
    static const ::delayInit::node caller_##Function(&Function);
}
