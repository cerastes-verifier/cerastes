// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_copy_ptr.c
// Pins: 0055 item 6: without p->data = NULL both structures hold one block at the return: two owned sites name it, "returns p->data and q->data as owned allocations, but they may be the same object" (S40 (e) e_ptr_copy_fail.c)
// Pre-registered from record 0055 item 6 (copies and values): permission at the closing brace (the return row; frozen base by hand: the same)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void move(struct buf *p, struct buf *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
{
  free(q->data);
  *q = *p;
}  // <- here
