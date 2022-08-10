#include "warn.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

void sc_warn(char const *msg)
{
#ifdef NDEBUG
    (void)msg;
#else
    fputs(msg, stderr);
    fputc('\n', stderr);
#endif
}
