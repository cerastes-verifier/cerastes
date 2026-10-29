// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/peek_reads.c
// Pins: 0037 item 10 and 0028 item 11: a function that only reads *p writes it: the write's permission check fails
#include <cerastes.h>
#include <stddef.h>

void put(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(cerastes_reads(*p))
{
  *p = 1;
}
