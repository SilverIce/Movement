
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
    vfprintf(file, str, ap);
    va_end(ap);

    printf( "\n" );
    fprintf(file, "\n" );

    fflush(file);
    fflush(stdout);
}
