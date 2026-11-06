// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 6b: the guard reads old(*q): the result is a block exactly where the value on entry was positive, whatever the function leaves in *q; the caller knows the entry value (c == 3)
// Pre-registered from record 0052 item 6b: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int *make(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == 0 && (cerastes_old(*q) > 0 ? cerastes_owns_block(r) : r == NULL))
{
  int v = *q;
  *q = 0;
  if (v > 0) {
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      abort();
    }
    *p = v;
    return p;
  }
  return NULL;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 0)
{
  int c = 3;
  int *p = make(&c);
  if (p != NULL) {
    free(p);
  }
  return c;
}
