// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2, audit round 1 S4: the route of loop_may_fill that is a store of a structure through memory (`*ppr = w;`, a Store of a struct value): a loop that has it, beside a struct local's owning leaf s.data that the loop does not name, with the block of s.data written before the loop: the flag survives the loop and the block is returned
// Pre-registered from record 0056 item 2 and audit S4 (build/m4.5d2b2c/t5/r2w.c on the L-1 build before filing): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct pair {
  int a;
  int b;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
  struct pair pr = { 0, 0 };
  struct pair *ppr = &pr;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    struct pair w = { i, i };
    *ppr = w;
    i = tick(i);
  }
  return s.data;
}
