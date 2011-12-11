#pragma once

namespace testing
{
    typedef void (*TestFn)(void);
    struct TestRegistrable {
        explicit TestRegistrable(TestFn testFuntionPtr, const char* name, const char* name2);
    };

    void _check(bool, const char* source, const char* expression);
}

#define TEST(name, name2) \
    void TESTCASE_NAME(name,name2)(void); \
    ::testing::TestRegistrable TestRegistrable_##name##_##name2(&TESTCASE_NAME(name,name2), #name, #name2); \
    void TESTCASE_NAME(name,name2)(void) // function body

#define TEST_DISABLED(name, name2) TEST(name, DISABLED_##name2)

#define TESTCASE_NAME(name, name2) testFunction_##name##_##name2

#define EXPECT_TRUE(expression) ::testing::_check(expression, __FUNCTION__, #expression);
#define EXPECT_EQ(a, b) EXPECT_TRUE( (a) == (b) );

#define EXPECT_THROW(expression, exception) \
    try { \
        expression; \
        ::testing::_check(false, __FUNCTION__, "expression '" #expression " does not throws '" #exception "' exception"); \
    } \
    catch( exception ) {}

#define EXPECT_NOTHROW(expression, exception) \
    try { \
        expression; \
    } catch( exception ) { \
        ::testing::_check(false, __FUNCTION__, "expression '" #expression "' throws '" #exception "' exception"); \
    }

