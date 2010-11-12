
#include "OutLog.h"

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

using namespace Movement;

static bool MOV_LOG_FILE_ENABLED     = false;
static bool MOV_LOG_CONSOLE_ENABLED  = false;

static FILE* file = NULL;

OutLogger::OutLogger()
{
    file = fopen("movement.log","wb");
    assert(file);

    write("    Log file initialized\n");
}

OutLogger::~OutLogger()
{
    write("\n    Log file closed");
    fclose(file);
    file = NULL;
}

void OutLogger::write(const char* str, ...)
{
    va_list ap;

    va_start(ap, str);

    if (MOV_LOG_CONSOLE_ENABLED)
        vfprintf(stdout, str, ap);
    if (MOV_LOG_FILE_ENABLED)
        vfprintf(file, str, ap);

    va_end(ap);

    if (MOV_LOG_FILE_ENABLED)
        fprintf(file, "\n" );

    if (MOV_LOG_CONSOLE_ENABLED)
        printf( "\n" );

    fflush(file);
    fflush(stdout);
}

