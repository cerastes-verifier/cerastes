// expect: error uninitialized @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_marked_member.c
// Pins: 0064 item 15 and 0058 item 4: the filler reads the marked buffer before writing it: the closure of the marked member promised no value: uninitialized at the read in the callee (the precedent negative/marked_field_alloc_fill_read_first.c)
// Pre-registered from record 0064 item 15 and the precedent marked_field_alloc_fill_read_first.c (uninitialized at the read in fill), before any run: unmeasured with an array member beside
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct buf {
  int a[4];
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

void fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->a[1] = 3;
  if (p->data != NULL) {
    int x = *p->data;  // <- here
    p->n = x;
  }
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
  b->data = (int *)malloc(sizeof(int));
  fill(b);
  buf_destroy(b);
  return 0;
}
