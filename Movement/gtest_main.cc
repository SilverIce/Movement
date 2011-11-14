
#include <iostream>

namespace testing{
    extern void RunAllTests();
}

int main(int argc, char **argv)
{
    std::cout << "Running main() from gtest_main.cc\n";

    testing::RunAllTests();
    return 0;
}
