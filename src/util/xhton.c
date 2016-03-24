#include <stdlib.h>
#include "util.h"




#ifdef PLATFORM_MIPS32
uint32_t b2ll(uint32_t blong)
{
    uint32_t res = 0;
    uint32_t b0, b1, b2, b3 = 0;

    b0 = (blong & 0xff) << 24;
    b1 = (blong & 0xff00) << 8;
    b2 = (blong & 0xff0000 ) >>8;
    b3 = (blong & 0xff000000) >> 24;

    return b0 | b1 | b2 | b3;

}

uint16_t b2ls(uint16_t bshort)
{
    uint16_t res = 0;
    uint16_t b0, b1 = 0;
    b0 = (bshort & 0xff) << 8;
    b1 = (bshort & 0xff00) >> 8;

    return b0 | b1;
}

uint32_t l2bl(uint32_t llong)
{
    uint32_t res = 0;
    uint32_t b0, b1, b2, b3 = 0;

    b0 = (llong & 0xff) << 24;
    b1 = (llong & 0xff00) << 8;
    b2 = (llong & 0xff0000 ) >>8;
    b3 = (llong & 0xff000000) >> 24;

    return b0 | b1 | b2 | b3;
}

uint16_t l2bs(uint16_t lshort)
{
    uint16_t res = 0;
    uint16_t b0, b1 = 0;
    b0 = (lshort & 0xff) << 8;
    b1 = (lshort & 0xff00) >> 8;

    return b0 | b1;
}


#endif

#ifdef PLATFORM_X86_64
uint32_t b2ll(uint32_t blong)
{
    return blong;
}

uint16_t b2ls(uint16_t bshort)
{
    return bshort;
}
#endif




