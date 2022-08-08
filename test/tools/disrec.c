#include "endian.h"
#include "fatal.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    MAXSIZE = 1024,
};

char msg[MAXSIZE] = {0};

int main(int argc, char **argv)
{
    if (argc != 2) fatal("Usage: disrec FILE");

    char const *filepath = argv[1];
    FILE *fp = fopen(filepath, "rb");
    if (!fp) fatal("could not open file");

    for (;;)
    {
        uint32_t size = 0;
        if (fread(&size, sizeof(size), 1, fp) != 1) break;
        size = musl_ntohl(size);
        if (fread(msg, 1, size, fp) != size) fatal("fread");
        printf("[%u] (%.*s)\n", size, (int)size, msg);
    }

    if (!feof(fp) || ferror(fp)) fatal("could not read file");

    fclose(fp);

    return 0;
}
