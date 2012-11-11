#pragma once

#include "typedefs.h"
#include "framework/meta.h"

namespace testing
{
    struct State;
    typedef void (*TestFn)(State&);
    struct TestInfo {
        TestFn function;
        const char* name;
        const char* name2;
    };

    inline TestInfo createInfo(TestFn function, const char* name, const char* name2) {
        TestInfo t = {function,name,name2};
        return t;
    }

    bool EXPORT runTests(const meta<TestInfo>::list& list);
    void EXPORT check(State& testState, bool, const char* source, const char* errorMessage);
}

#define TEST_MIXTURE(MixtureType) \
    TEST(mixture, MixtureType) { \
        MixtureType mix; \
        mix.testing::mixture::__teststate = __teststate; \
        mix.MixtureType::test(); \
    }

#define TEST(name, name2) \
    void TESTCASE_NAME(name,name2)(::testing::State&); \
    static const meta<::testing::TestInfo> testInfo_##name##_##name2( \
        ::testing::createInfo(&TESTCASE_NAME(name,name2),#name,#name2)); \
    void TESTCASE_NAME(name,name2)(::testing::State& testState)

#define TEST_DISABLED(name, name2) TEST(name, DISABLED_##name2)

#define TESTCASE_NAME(name, name2) TestFunc_##name##_##name2


#define TEST_REGISTER(function) \
    TEST(function, none) { extern void function(::testing::State&); function(testState); }

#define EXPECT_TRUE(expression) ::testing::check(testState, expression, __FUNCTION__, #expression " is false");
#define EXPECT_FALSE(expression)  ::testing::check(testState, !(expression), __FUNCTION__, #expression " is true");
#define EXPECT_EQ(a, b) ::testing::check(testState, (a) == (b), __FUNCTION__, #a " != " #b);

#define EXPECT_THROW(expression, exception) \
    try { \
        expression; \
        ::testing::check(testState, false, __FUNCTION__, "'" #expression "' does not throws nor '" #exception "' nor any other exception"); \
    } \
    catch( const exception& ) {;} \
    catch(...) { \
        ::testing::check(testState, false, __FUNCTION__,  "'" #expression "' does not throws '" #exception "' exception, but throws unknown exception"); \
        throw; \
    }

#define EXPECT_NOTHROW(expression) \
    try { \
        expression; \
    } catch(...) { \
        ::testing::check(testState, false, __FUNCTION__, "'" #expression "' throws exception"); \
    }

