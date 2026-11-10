// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 2, 3: cerastes::owning on a pointer field (trailing position): the initializer (a must_init parameter: it needs requires(owns(*p)), 0042 item 4) gives the structure and its field's block (a given site), the destructor consumes owns_block(p) with the closure of the field, and main allocates, initializes and destroys
// Pre-registered from record 0055 items 2, 3: verifies in both modes (the closure of each atom is p->data == NULL || owns_block(p->data) under the atom's own guard; written by hand it verifies on the d2b2a encoder)
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
