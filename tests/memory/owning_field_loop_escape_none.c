// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 11 (question 209): s.data is NULL at the loop's entry, the loop stores a pointer of its type through memory (`*pz = pk;`), and the leaf is read after the loop under a test: the struct local's address is formed only as the base of member accesses, so it does not escape and the loop does not reach the leaf; the base reports uninitialized at the read, a false alarm of the by-type reach
// Pre-registered from record 0056 item 11 and audit round 2 S2 (probe build/m4.5d2b2c/t7/esc_none.c: uninitialized at the read on the L-1 build; not yet run on the build with 209): verifies in both modes
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
  int k = 0;
  int *pk = &k;
  int *z = NULL;
  int **pz = &z;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pz = pk;
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
