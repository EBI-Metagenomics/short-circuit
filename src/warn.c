#include "warn.h"
#include <stdio.h>

void warn(char const *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
}
