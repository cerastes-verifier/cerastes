// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2, audit round 2 B1 (store route): the model of owning_field_loop_site_store_null_unwritten.c with the block written in the loop: an int is stored, so the flag is kept and the read after the loop has a value
// Pre-registered from record 0056 item 2 (probe build/m4.5d2b2c/t7/store_w.c on the L-1 build): verifies in both modes
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
  struct buf s;
  s.n = 0;
  s.data = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = i;
    }
    i = tick(i);
  }
  int x = 0;
  if (s.data != NULL) {
    x = *s.data;
  }
  free(s.data);
  s.data = NULL;
  return x;
}
