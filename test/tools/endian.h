#ifndef TEST_TOOLS_ENDIAN_H
#define TEST_TOOLS_ENDIAN_H

#include <stdint.h>

static inline uint32_t musl_bswap_32(uint32_t __x)
{
    return __x >> 24 | (__x >> 8 & 0xff00) | (__x << 8 & 0xff0000) | __x << 24;
}

static inline uint32_t musl_htonl(uint32_t n)
{
    union
    {
        int i;
        char c;
    } u = {1};
    return u.c ? musl_bswap_32(n) : n;
}

#endif
