#include "gtest.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <intrin.h>
#include <set>

namespace testing
{
    struct TestInfo
    {
        TestFn testFn;
        const char * Name;
        const char * Name2;

        static bool Disabled(const TestInfo* info) {
            return strncmp(info->Name2, "DISABLED", 8) == 0;
        }

        static TestInfo* create(TestFn testFuntionPtr, const char* name, const char* name2) {
            TestInfo* test = new TestInfo();
            test->testFn = testFuntionPtr;
            test->Name = name;
            test->Name2 = name2;
            return test;
        }

        static bool Compare(const TestInfo* left, const TestInfo* right) {
            return strcmp(left->Name, right->Name) == -1;
        }

        static void Delete(TestInfo* test) {
            delete test;
        }

        static void InvokeTest(TestInfo* test)
        {
            if (!TestInfo::Disabled(test)) {
                printf("\n    %s::%s has been invoked\n", test->Name, test->Name2);
                test->testFn();
            }
            else
                printf("\n    %s::%s is disabled\n", test->Name, test->Name2);
        }
    };

    struct TestRegistry
    {
        std::vector<TestInfo*> tests;

        ~TestRegistry() {
            std::for_each(tests.begin(),tests.end(),TestInfo::Delete);
        }

        size_t totalAmount() {
            return tests.size();
        }

        void AddTest(TestInfo* test){
            tests.push_back(test);
        }

        static TestRegistry& instance() {
            static TestRegistry reg;
            return reg;
        }
    };

    TestRegistrable::TestRegistrable(TestFn testFuntionPtr, const char* name, const char* name2)
    {
        TestRegistry::instance().AddTest( TestInfo::create(testFuntionPtr,name,name2) );
    }

    struct Statistics
    {
        //int countTestsFailed;
        size_t countChecksFailed;
        size_t countDisabledTests;
        std::set<TestInfo*> testsFailed;

        size_t countTestsFailed() {
            return testsFailed.size();
        }

        explicit Statistics() {
            countChecksFailed = 0;
            countDisabledTests = 0;
        }

        void OnCheckFailed(TestInfo* test) {
            ++countChecksFailed;
            testsFailed.insert(test);
        }
    };

    struct TestRunner
    {
        Statistics statistics;
        TestInfo *currentTest;

        TestRunner() {
            currentTest = NULL;
        }

        void OnCheckFailed() {
            statistics.OnCheckFailed(currentTest);
        }

        void RunAllTests()
        {
            std::vector<TestInfo*>& tests = TestRegistry::instance().tests;
            // No need sort tests: their natural order is important. Tests from the same compile unit will be grouped together
            //std::sort(tests.begin(),tests.end(),TestInfo::Compare);
            statistics.countDisabledTests = std::count_if(tests.begin(),tests.end(),TestInfo::Disabled);

            for(std::vector<TestInfo*>::iterator it = tests.begin(); it!=tests.end(); ++it) {
                currentTest = *it;
                TestInfo::InvokeTest(currentTest);
                currentTest = NULL;
            }

            OnTestsComplete();
        }

        void OnTestsComplete() {
            printf("\n");
            printf("%u tests failed\n", statistics.countTestsFailed());
            printf("%u tests disabled\n", statistics.countDisabledTests);
            printf("%u tests total amount\n", TestRegistry::instance().totalAmount());
        }

        static TestRunner& instance() {
            static TestRunner reg;
            return reg;
        }
    };

    bool BREAK_ON_TEST_FAIL = true;

    void _check(bool result, const char* expression, const char* function)
    {
        //EXPECT_TRUE(TestRunner::instance().currentTest != NULL);

        if (result)
            return;

        printf("In '%s': expression '%s' failed!\n", function, expression);

        TestRunner::instance().OnCheckFailed();
        if (BREAK_ON_TEST_FAIL)
            __debugbreak();
    }

    void RunAllTests()
    {
        TestRunner::instance().RunAllTests();
    }
}
