#pragma once

#include "typedefs.h"
#include <intrin.h>

namespace Movement
{
    extern void log_write(const char* fmt, ...);
    extern void log_console(const char* str, ...);
    extern void log_write_trace();

#define CountOf(array) (sizeof(array)/sizeof(array[0]))

#ifndef static_assert
    #define CONCAT(x, y) CONCAT1 (x, y)
    #define CONCAT1(x, y) x##y
    #define static_assert(expr, msg) typedef char CONCAT(static_assert_failed_at_line_, __LINE__) [(expr) ? 1 : -1]
#endif

#define mov_assert(expr) \
    if (!(expr)){ \
        log_write("In "__FUNCTION__": assertion '"#expr"' failed"); \
        __debugbreak(); \
    }

/** Use it to validate object state */
#define assert_state(expr) mov_assert(expr)

/** Use it to validate object state */
#define assert_state_msg(expr, msg, ...) \
    if (bool(expr) == false) { \
        log_write("In "__FUNCTION__" assertion '"#expr"' failed:"); \
        log_write("    " msg, __VA_ARGS__); \
        __debugbreak(); \
    }

#define log_function(msg, ...)  log_write(__FUNCTION__ ": " msg, __VA_ARGS__) \

#define log_fatal(msg, ...) { \
        log_write(__FUNCTION__ ": " msg, __VA_ARGS__); \
        __debugbreak(); \
    }

#define check(expr) \
    if (bool(expr) == false) { \
        log_write("In "__FUNCTION__": check '"#expr"' failed"); \
        __debugbreak(); \
    }

    template<class T, T limit>
    class counter
    {
    public:
        counter() { init();}

        void Increase()
        {
            if (m_counter == limit)
                init();
            else
                ++m_counter;
        }

        T NewId() { Increase(); return m_counter;}
        T getCurrent() const { return m_counter;}

        static const T Limit = limit;

    private:
        void init() { m_counter = 0; }
        T m_counter;
    };

    typedef counter<uint32, 0xFFFFFFFF> UInt32Counter;
}
