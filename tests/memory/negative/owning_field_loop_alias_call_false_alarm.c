// expect: error uninitialized @ L34
// expect: error permission @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_free_after.c
// Pins: 0055 item 7 (a false alarm pinned, re-audit N3): a loop that passes the struct local through an alias, ps = &s, to a borrower that frees and replaces the field: the alias gives the loop no place for the struct local's leaf (the alias is a heap place of a struct pointer, which does not carry its pointee's closure); without the alias (owning_field_loop_free_after.c shape) it verifies. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (probes r4/ and r5/), observed on the session build after the fixes: d_alias_call.c: the expect lines are the actual outcome, recorded as a pin
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
