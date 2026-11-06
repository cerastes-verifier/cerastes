// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (target v, amends 0029 item 1): an automatic pointer local marked cerastes::owning that receives a block and frees it (at the end of a function and inside a nested block): its lifetime-end row finds no token held
// Pre-registered from record 0052 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

int use_and_free(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  free(p);
  return v;
}

int block_scope(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  {
    CERASTES_OWNING int *q = (int *)malloc(sizeof(int));
    if (q == NULL) {
      return -1;
    }
    *q = c;
    free(q);
  }
  return 0;
}

int account_roundtrip(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct account *a = (struct account *)calloc(1, sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  int b = a->balance;
  free(a);
  return b;
}
