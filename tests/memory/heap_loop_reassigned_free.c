// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 6 and 10's follow-up (re-audit S3, M4.5d2v): a pointer local the loop assigns (c = a) is kept well-formed by the loop's automatic invariant, and the block stored in a member is freed through the member after the loop
#include <cerastes.h>
#include <stdlib.h>

struct L {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct L *l = (struct L *)malloc(sizeof(struct L));
  if (l == NULL) {
    return -1;
  }
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    free(l);
    return -1;
  }
  l->q = a;
  int *c = a;
  *c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    c = a;
    *c = i;
    i += 1;
  }
  free(l->q);
  free(l);
  return 0;
}
