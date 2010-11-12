#pragma once


namespace Movement
{
    class OutLogger
    {
    public:

        explicit OutLogger();
        ~OutLogger();

        void write(const char* str, ...);
    };

    static OutLogger movLog;
}

