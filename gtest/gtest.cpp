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
        size_t countFailedTests;

        size_t countTestsFailed() {
            return countFailedTests;
        }

        explicit Statistics() {
            countChecksFailed = 0;
            countDisabledTests = 0;
            countFailedTests = 0;
        }

        void OnCheckFailed() {
            ++countChecksFailed;
        }
        void OnTestFailed() {
            ++countFailedTests;
        }
    };

    struct TestRunner
    {
        Statistics statistics;
        TestInfo *currentTest;

        TestRunner() {
            currentTest = NULL;
        }

        bool RunAllTests()
        {
            std::vector<TestInfo*>& tests = TestRegistry::instance().tests;
            // No need sort tests: their natural order is important. Tests from the same compile unit will be grouped together
            //std::sort(tests.begin(),tests.end(),TestInfo::Compare);
            statistics.countDisabledTests = std::count_if(tests.begin(),tests.end(),TestInfo::Disabled);

            for(std::vector<TestInfo*>::iterator it = tests.begin(); it!=tests.end(); ++it)
                InvokeTest(*it);

            OnTestsComplete();

            return statistics.countTestsFailed() == 0;
        }

        void OnTestsComplete() {
            printf("\n");
            printf("%u tests failed\n", statistics.countTestsFailed());
            printf("%u tests disabled\n", statistics.countDisabledTests);
            printf("%u tests total amount\n", TestRegistry::instance().totalAmount());
        }

        void InvokeTest(TestInfo * test)
        {
            currentTest = test;
            try {
                if (TestInfo::Disabled(test)) 
                    printf("\n    %s::%s is disabled\n", test->Name, test->Name2);
                else {
                    printf("\n    %s::%s has been invoked\n", test->Name, test->Name2);
                    test->testFn();
                    if (test->isFailed)
                        printf("\n    %s::%s has been failed!\n", test->Name, test->Name2);
                }
            }
            catch (...) {
                _check(false, "current test", "exception wasn't expected");
            }
            currentTest = NULL;
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

    bool RunAllTests()
    {
        return TestRunner::instance().RunAllTests();
    }
}
