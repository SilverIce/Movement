
#include "OutLog.h"

#include <assert.h>
#include <stdarg.h>


OutLogger::OutLogger() : file(0)
{
    file = fopen("out.log","wb");
    assert(file);

    write("    Init log file\n");
}

OutLogger::~OutLogger()
{
    write("\n    Log file closed");
    fclose(file);
}

void OutLogger::write(const char* str, ...)
{
    va_list ap;

    va_start(ap, str);
    vfprintf(stdout, str, ap);
    printf( "\n" );
    va_end(ap);

    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n" );
    va_end(ap);
    fflush(file);
    fflush(stdout);
}
