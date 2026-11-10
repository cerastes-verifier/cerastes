// expect: error permission @ L30
// mode: c23, c++20
// twin: tests/memory/owning_field_copy_null.c
// Pins: 0055 item 6: after t = s; both structures are handed to one call that owns both: two sites name one block, "passes a->data and b->data to both, but they may be the same object" (S40 (e) e_both_fail.c)
// Pre-registered from record 0055 item 6 (copies and values): permission at the call (today's call-site row on two owned sites; frozen base by hand: permission at the call)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void both(struct buf *a, struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b))
{
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 1 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  struct buf t;
  t = s;
  both(&s, &t);  // <- here
  free(t.data);
  return 0;
}
