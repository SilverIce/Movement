#pragma once

#include <stdio.h>


namespace Movement
{
    class OutLogger
    {
    public:

        explicit OutLogger();
        ~OutLogger();

        void write(const char* str, ...);

    private:
        FILE* file;
    };

    static OutLogger movLog;
}

