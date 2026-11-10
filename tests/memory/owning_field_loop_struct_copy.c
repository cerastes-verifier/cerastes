// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2 (the structure-copy route, audit round 2 / edg-backend report 4): a struct local whose owning leaf is NULL at the loop's entry is overwritten by a copy of another struct local in the loop (`s = t;`, which the encoder expands into a store of the pointer leaf), the copy brings a malloc'd block that is never written, and nothing reads it: the leaf is not claimed to have a value, and the loop verifies
// Pre-registered from edg-backend report 4 (F/c/ss6.c: verified on the base and on the build with 208-211, 213): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int f(void)
{
  struct buf s = { NULL };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    struct buf t = { NULL };
    t.data = (int *)malloc(sizeof(int));
    free(s.data);
    s = t;
    t.data = NULL;
    i = tick(i);
  }
  free(s.data);
  s.data = NULL;
  return 0;
}
