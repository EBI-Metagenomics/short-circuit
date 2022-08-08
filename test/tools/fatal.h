#ifndef TEST_TOOLS_FATAL_H
#define TEST_TOOLS_FATAL_H

#include <stdio.h>
#include <stdlib.h>

static void fatal(char const *msg)
{
    fputs("FATAL: ", stderr);
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

#endif
