#include <stdio.h>
#include <stdarg.h>
#include <time.h>

namespace Movement{

    static bool MOV_LOG_FILE_ENABLED     = true;
    static bool MOV_LOG_CONSOLE_ENABLED  = true;

    struct __log_init 
    {
        __log_init()
        {
            file = fopen("movement.log","a++");

            fprintf(file, "Movement Log\n");
            time_t t;
            time(&t);
            fprintf(file, "Start: %s\n", ctime(&t));
            fflush(file);
        }

        ~__log_init()
        {
            time_t t;
            time(&t);
            fprintf(file, "Shutdown: %s\n", ctime(&t));
            fclose(file);
        }

        FILE* file;

    } static log;
    
    void log_write(const char* str, ...)
    {
        va_list ap;

        va_start(ap, str);

        if (MOV_LOG_CONSOLE_ENABLED)
            vfprintf(stdout, str, ap);
        if (MOV_LOG_FILE_ENABLED && log.file)
            vfprintf(log.file, str, ap);

        va_end(ap);

        if (MOV_LOG_FILE_ENABLED && log.file)
            fprintf(log.file, "\n" );

        if (MOV_LOG_CONSOLE_ENABLED)
            printf( "\n" );

        fflush(log.file);
        fflush(stdout);
    }

    void log_console(const char* str, ...)
    {
        va_list ap;
        va_start(ap, str);
        vfprintf(stdout, str, ap);
        va_end(ap);
        printf( "\n" );
        fflush(stdout);
    }
}


