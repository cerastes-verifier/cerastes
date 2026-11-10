// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_replace.c
// Pins: 0055 item 4 (the value rule): the borrower stores the address of a local into the owning field it has just freed; the store, not the return, is where "owning field p->data receives a pointer that is not the start of a block the function holds" is reported; the field is set back to NULL before the return so that the closure's return row holds
// Pre-registered from record 0055 item 4: permission at the store (today's encoder, with the clause written by hand, verifies this program: the value rule is new)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void replace(struct buf *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int x = v;
  free(p->data);
  p->data = &x;  // <- here
  p->data = NULL;
}

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  replace(b, 3);
  buf_destroy(b);
  return 0;
}
