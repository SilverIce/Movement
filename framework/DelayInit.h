#pragma once

#include "framework/meta.h"

namespace delayInit
{
    /** This structure should not be used directly by user.
        Also it should be allocated in global scope only. */
    typedef void (*CTor)();
    template<class T> struct Obj {
        T * _objPtr;
        char _data[sizeof(T)];
        inline Obj() : _objPtr(0) {}
        inline ~Obj() { if (_objPtr) _objPtr->~T();}
    };

    // --- BEGIN public API ---

    /** Launches delayed initialization. Function asserts that it was not called before. */
    inline void callCtors() {
        meta<CTor> * first = meta<CTor>::getListConst().first;
        while(first) {
            (*first->info)();
            first = first->next;
        }
    }

#define DELAYED_INIT2(Type, name, ...) \
    static void FuncCtor_##name() { \
        static ::delayInit::Obj<Type > s_instance; \
        name = s_instance._objPtr = new (s_instance._data) Type(__VA_ARGS__); \
    } \
    DELAYED_CALL(FuncCtor_##name);

#define DELAYED_INIT(Type, name, ...) \
    static void FuncCtor_##name() { \
        static ::delayInit::Obj<Type > s_instance; \
        s_instance._objPtr = new (s_instance._data) Type(__VA_ARGS__); \
    } \
    DELAYED_CALL(FuncCtor_##name);

#define DELAYED_CALL_ARGS(Function, shortname, ...) \
    static void FuncCaller_##shortname() { \
        Function(__VA_ARGS__); \
    } \
    static const ::meta<::delayInit::CTor> caller_##shortname(&FuncCaller_##shortname);

#define DELAYED_CALL(Function) \
    static const ::meta<::delayInit::CTor> caller_##Function(&Function);

    // --- END public API ---
}
