// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0055 item 3 ("Merging and allocators"; 0054 item 5): the user writes the clause beside owns(*p) with cerastes_owns_vlibc_block, which takes the closure's place; the structure and the field's block come from vlibc's allocator and are freed with vlibc_free
// Pre-registered from record 0055 item 3: verifies (the merged clause names vlibc's allocator; the closure adds nothing)
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
  vlibc_free(p->data);
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
