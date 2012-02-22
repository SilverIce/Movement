#pragma once

#include "framework/DelayInit.h"

namespace testing
{
    typedef void (*TestFn)(void);
    void RegisterTest(TestFn testFuntionPtr, const char* name, const char* name2);

    void _check(bool, const char* source, const char* expression);
}

#ifndef GTEST_DISABLED
    #define TEST(name, name2) \
        void TESTCASE_NAME(name,name2)(void); \
        DELAYED_CALL_ARGS(::testing::RegisterTest, RegTest_##name##_##name2, TESTCASE_NAME(name,name2), #name, #name2); \
        void TESTCASE_NAME(name,name2)(void) // function body
#else
    #define TEST(name, name2) \
        void TESTCASE_NAME(name,name2)(void)
#endif

#define TEST_DISABLED(name, name2) TEST(name, DISABLED_##name2)

#define TESTCASE_NAME(name, name2) TestFunc_##name##_##name2

#define TEST_REGISTER(function) \
        void function(); \
        DELAYED_CALL_ARGS(::testing::RegisterTest,RegTest_##function, function, #function, "");

#define EXPECT_TRUE(expression) ::testing::_check(expression, __FUNCTION__, #expression);
#define EXPECT_EQ(a, b) EXPECT_TRUE( (a) == (b) );

#define EXPECT_THROW(expression, exception) \
    try { \
        expression; \
        ::testing::_check(false, __FUNCTION__, "expression '" #expression " does not throws '" #exception "' exception"); \
    } \
    catch( const exception& ) {}

#define EXPECT_NOTHROW(expression) \
    try { \
        expression; \
    } catch(...) { \
        ::testing::_check(false, __FUNCTION__, "expression '" #expression "' throws exception"); \
    }

