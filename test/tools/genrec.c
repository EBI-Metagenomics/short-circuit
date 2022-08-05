#include "endian.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fatal(char const *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

static char const paragraph[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";

char *lorem_new(unsigned long size)
{
    char *lorem = malloc(size + 1);
    if (!lorem) exit(1);

    for (unsigned long i = 0; i < size; ++i)
        lorem[i] = paragraph[i % (sizeof(paragraph) - 1)];

    lorem[size] = 0;

    return lorem;
}

enum cmd
{
    PRINT_SIZE,
    PRINT_DATA,
    PRINT_BOTH,
    INVALID,
};

enum cmd parse_cmd(char *cmd)
{
    char names[][16] = {"print-size", "print-data", "print-both"};
    enum cmd cmds[] = {PRINT_SIZE, PRINT_DATA, PRINT_BOTH};
    for (unsigned i = 0; i < 3; ++i)
    {

        unsigned len = (unsigned)strlen(cmd);
        if (strlen(names[i]) == len && !strncmp(names[i], cmd, len))
            return cmds[i];
    }
    return INVALID;
}

unsigned long parse_size(char const *num)
{
    char *end = 0;
    long size = strtol(num, &end, 10);
    if (size < 0 || size > LONG_MAX) fatal("size outside range");
    if (end == num) fatal("failed to parse integer");
    return (unsigned long)size;
}

void print_data(unsigned long size)
{
    char *data = lorem_new(size);
    if (!data) fatal("not enough memory");
    fputs(data, stdout);
    free(data);
}

void print_size(unsigned long size)
{
    uint32_t sz = (uint32_t)size;
    sz = musl_htonl(sz);
    fwrite(&sz, 1, 4, stdout);
}

void print_both(unsigned long size)
{
    print_size(size);
    print_data(size);
}

int main(int argc, char **argv)
{
    if (argc != 3)
        fatal("Usage: generate_record print-size|print-data|print-both SIZE");

    enum cmd cmd = parse_cmd(argv[1]);
    unsigned long size = parse_size(argv[2]);

    if (cmd == INVALID) fatal("invalid command");
    if (cmd == PRINT_SIZE) print_size(size);
    if (cmd == PRINT_DATA) print_data(size);
    if (cmd == PRINT_BOTH) print_both(size);

    return 0;
}
