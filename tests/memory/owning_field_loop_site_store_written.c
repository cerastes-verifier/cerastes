// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 11 (question 209, the escape scan): the block of a struct local's leaf is written before a loop that stores a pointer of the leaf's type through memory (`*pz = pk;`): the struct local's address is never formed except as the base of a member access, so the loop does not reach s.data and its flag is not touched
// Pre-registered from audit round 2 S2 and question 209 (the former pin owning_field_loop_site_store_written_false_alarm.c; not yet run on the build with 209): verifies in both modes
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

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
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
  return s.data;
}
