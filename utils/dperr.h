#ifndef _DPERR_H_
#define _DPERR_H_

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

void dperr(const char * prompt, ...)
{
    va_list args;
    va_start(args, prompt);
    vprintf(prompt, args);
    va_end(args);
    printf(": %s\n", strerror(errno));
}

#endif
