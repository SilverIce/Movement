#include "gtest.h"
#include <QtCore/QVector>
#include <algorithm>
#include <stdio.h>
#include <intrin.h>

namespace testing
{
    struct TestInfo
    {
        TestFn testFn;
        const char * Name;
        const char * Name2;
        bool isFailed;

        static bool Disabled(const TestInfo* info) {
            return strncmp(info->Name2, "DISABLED", 8) == 0;
        }

        static TestInfo* create(TestFn testFuntionPtr, const char* name, const char* name2) {
            TestInfo* test = new TestInfo();
            test->testFn = testFuntionPtr;
            test->Name = name;
            test->Name2 = name2;
            test->isFailed = false;
            return test;
        }

        static bool Compare(const TestInfo* left, const TestInfo* right) {
            return strcmp(left->Name, right->Name) == -1;
        }

        static void Delete(TestInfo* test) {
            delete test;
        }
    };

    struct TestRegistry
    {
        QVector<TestInfo*> tests;

        ~TestRegistry() {
            Clear();
        }

        int totalAmount() {
            return tests.size();
        }

        void AddTest(TestInfo* test){
            tests.push_back(test);
        }

        void Clear() {
            qDeleteAll(tests);
            tests.clear();
        }

        static TestRegistry& instance() {
            static TestRegistry reg;
            return reg;
        }
    };

    void RegisterTest(TestFn testFuntionPtr, const char* name, const char* name2)
    {
        TestRegistry::instance().AddTest( TestInfo::create(testFuntionPtr,name,name2) );
    }

    struct Statistics
    {
        //int countTestsFailed;
        size_t countChecksFailed;
        size_t countDisabledTests;
        size_t countFailedTests;
        size_t countTotalTests;

        explicit Statistics() {
            countChecksFailed = 0;
            countDisabledTests = 0;
            countFailedTests = 0;
            countTotalTests = 0;
        }

        void OnCheckFailed() {
            ++countChecksFailed;
        }
        void OnTestFailed() {
            ++countFailedTests;
        }

        void OnTestsComplete() {
            printf("\n");
            printf("%u tests failed\n", countFailedTests);
            printf("%u tests disabled\n", countDisabledTests);
            printf("%u tests total amount\n", countTotalTests);
        }
    };

    struct TestRunner
    {
        Statistics statistics;
        TestInfo *currentTest;

        TestRunner() {
            currentTest = NULL;
        }

        bool RunAllTests(const QVector<TestInfo*>& tests)
        {
            // No need sort tests: their natural order is important. Tests from the same compile unit will be grouped together
            //std::sort(tests.begin(),tests.end(),TestInfo::Compare);
            statistics.countDisabledTests = std::count_if(tests.begin(),tests.end(),TestInfo::Disabled);
            statistics.countTotalTests = tests.size();

            foreach(TestInfo* test, tests)
                InvokeTest(test);

            statistics.OnTestsComplete();

            return statistics.countFailedTests == 0;
        }

        void InvokeTest(TestInfo * test)
        {
            if (TestInfo::Disabled(test)) {
                printf("    %s::%s is disabled\n", test->Name, test->Name2);
                return;
            }

            currentTest = test;
            printf("    %s::%s has been invoked\n", test->Name, test->Name2);
            {
                EXPECT_NOTHROW( test->testFn() );
            }
            currentTest = NULL;

            if (test->isFailed)
                printf("    %s::%s has been failed!\n", test->Name, test->Name2);
        }

        void OnCheckFailed()
        {
            statistics.OnCheckFailed();
            if (!currentTest->isFailed) {
                currentTest->isFailed = true;
                statistics.OnTestFailed();
            }
        }

        static TestRunner& instance() {
            static TestRunner reg;
            return reg;
        }
    };

    bool BREAK_ON_TEST_FAIL = true;
    bool DETAILED_OUTPUT = true;

    void _check(bool result, const char* source, const char* expression)
    {
        if (result)
            return;

        printf("In '%s': expression '%s' failed!\n", source, expression);

        TestRunner::instance().OnCheckFailed();
        if (BREAK_ON_TEST_FAIL)
            __debugbreak();
    }

    /** Returns true in case all tests succeed */
    bool RunAllTests()
    {
        bool succeed = TestRunner::instance().RunAllTests(TestRegistry::instance().tests);
        TestRegistry::instance().Clear();
        return succeed;
    }

    TEST(gtest, test_self)
    {
        EXPECT_TRUE( true );
        EXPECT_EQ( 1, 1);

        EXPECT_THROW( throw "expected_exception", char* );
        EXPECT_NOTHROW( ; );
    }

    TEST_DISABLED(gtest, disabled)
    {
        EXPECT_TRUE( false );
    }
}
