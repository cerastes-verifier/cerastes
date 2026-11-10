// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 4 (limit 2): the same loop with refill(&s), the address of the struct local named in the call: the base reports uninitialized at the call although the owning_field_alias_fill.c shape verifies; the leaf's ghost is refreshed for the callee's closure
// Pre-registered from record 0056 item 4 and S41 (0) run 2 (not yet run here): verifies in both modes (on the frozen base: uninitialized at the call, the line of refill(&s))
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void refill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 1;
  }
}

int f(void)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    refill(&s);
    i = i + 1;
  }
  free(s.data);
  s.data = NULL;
  return 0;
}
