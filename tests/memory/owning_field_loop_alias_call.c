// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 4 (limit 2): a loop that passes the struct local through an alias, ps = &s, to a borrower that frees and replaces the field, and frees the field after the loop: the leaf's ghost is refreshed after a call through the alias and the leaf's site knows the callee takes the owner
// Pre-registered from record 0056 item 4 (not yet run): verifies in both modes; it is the former pin owning_field_loop_alias_call_false_alarm.c (uninitialized @ L34 and permission @ L36 on the base) with a reworded header
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
  struct buf *ps = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    refill(ps);
    i = i + 1;
  }
  free(s.data);
  s.data = NULL;
  return 0;
}
