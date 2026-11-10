// expect: error unsequenced @ L28
// mode: c23, c++20
// twin: tests/memory/owning_field_seq_call.c
// Pins: 0055 item 3 (S1): the read *t and the call refill(p) in one expression: the call frees the block *t reads, and C does not order them (0039): unsequenced, at the later call (t/seq2.c)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), unsequenced at the call (t/seq2.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int refill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  free(p->data);
  p->data = NULL;
  return 0;
}

int total(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data != NULL)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int *t = p->data;
  int x = *t + refill(p);  // <- here
  return x;
}
