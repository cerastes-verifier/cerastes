// expect: error invalid-free @ L19
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/owning_field_vlibc.c
// Pins: 0055 item 3 ("Merging and allocators"; 0050 item 3): the merged clause says the field's block is vlibc's; freeing it with the platform's free is invalid-free
// Pre-registered from record 0055 item 3: invalid-free at the free call (as owns_block_load_vlibc_free_mismatch.c)
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void renew(struct buf *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p) && (p->data == NULL || cerastes_owns_vlibc_block(p->data)))
    CERASTES_ENSURES(cerastes_owns(*p) && (p->data == NULL || cerastes_owns_vlibc_block(p->data)))
{
  free(p->data);  // <- here
  p->data = (int *)vlibc_malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = v;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)vlibc_calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  renew(b, 2);
  vlibc_free(b->data);
  vlibc_free(b);
  return 0;
}
