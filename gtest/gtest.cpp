#include "gtest.h"
#include <vector>
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

        static TestInfo* create(TestFn testFuntionPtr, const char* name, const char* name2) {
            TestInfo* test = new TestInfo();
            test->testFn = testFuntionPtr;
            test->Name = name;
            test->Name2 = name2;
            return test;
        }

        static void Invoke(TestInfo* test) {
            printf("%s::%s has been invoked\n\n", test->Name, test->Name2);
            test->testFn();
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
        std::vector<TestInfo*> tests;

        ~TestRegistry() {
            std::for_each(tests.begin(),tests.end(),TestInfo::Delete);
        }

        void AddTest(TestInfo* test){
            tests.push_back(test);
        }

        void InvokeAll() {
            std::sort(tests.begin(),tests.end(),TestInfo::Compare);
            std::for_each(tests.begin(),tests.end(),TestInfo::Invoke);
        }

        static TestRegistry& instance() {
            static TestRegistry reg;
            return reg;
        }
    };

    TestRegistrable::TestRegistrable(TestFn testFuntionPtr, const char* name, const char* name2)
    {
        TestInfo* inf = TestInfo::create(testFuntionPtr,name,name2);
        TestRegistry::instance().AddTest(inf);
    }

    void RunAllTests()
    {
        TestRegistry::instance().InvokeAll();
    }

    void _check(bool result, const char* expression)
    {
        if (result)
            return;

        printf("%s\n", expression);
        __debugbreak();
    }
}
