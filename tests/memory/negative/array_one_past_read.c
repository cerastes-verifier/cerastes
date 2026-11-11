// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/array_one_past.c
// Pins: 0059 item 4 (question 229): a read through the one-past pointer: permission (*p keeps today's rows)
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int one_past(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  int *e = &a[3];
  return *e;
}
