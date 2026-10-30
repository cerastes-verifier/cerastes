// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/conditional_ownership.c
// Pins: 0039 item 7: the write sits under !c, where the contract only reads *p: the write's permission check fails
#include <cerastes.h>
#include <stddef.h>

int step(int c, int *p)
    CERASTES_REQUIRES((c ? cerastes_owns(*p) : cerastes_reads(*p)) && *p >= 0 && *p < 100)
    CERASTES_ENSURES(r, (c ? cerastes_owns(*p) : cerastes_reads(*p)) && r == *p)
{
  if (!c) {
    *p += 1;
  }
  return *p;
}
