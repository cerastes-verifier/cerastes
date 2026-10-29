// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/peek_reads.c
// Pins: 0037 item 10: the callee hands part of its borrowed fraction to a callee that keeps it and returns less than it promised
#include <cerastes.h>
#include <stddef.h>

void consume(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p));

void hold(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(cerastes_reads(*p))
{
  consume(p);
}
