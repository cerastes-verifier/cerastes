// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/owning_field_plain_loop.c
// Pins: 0055 items 5, 7: the same program with the struct local holding a block across the loop and never freed: the lifetime-end row at the return
// Pre-registered from audit round 2 on Parts B/C (probes r3/): record 0055 item 5: leak at the return (new; plain_loop_noglobal.c is the positive)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int f(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = tick(i);
  }
  return s.n;  // <- here
}
