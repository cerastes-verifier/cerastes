// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 items 3, 4 (the value rule): the initializer stores the address of a local into the owning field, which is not a block the function holds (the field is set back to NULL right after, so that the closure's return row holds and the store is the only error): "owning field p->data receives a pointer that is not the start of a block the function holds"; the closure clause it gives the caller would otherwise claim the local as an owned block
// Pre-registered from record 0055 item 4: permission at the store (the frozen base, with the clause written by hand and the same NULL store, verifies this program: the value rule is new)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_init(struct buf *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int x = 0;
  p->data = &x;  // <- here
  p->data = NULL;
  p->n = 0;
}

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int main(void)
{
  struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return 1;
  }
  buf_init(b);
  buf_destroy(b);
  return 0;
}
