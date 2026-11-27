// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (293, 294; amends 0058 item 4; the pin struct_array_reject_marked_members.c's R is struct R, f_d07 is kept): a ref_to_uninit member beside an array member (plain, a no-op in the default mode), and an owning marked member beside one: the structure is allocated, its buffer allocated and not yet written, a filler writes an element and the buffer
// Pre-registered from record 0064 item 15 and the precedents marked_field_plain.c and marked_field_alloc_fill.c, before any run: verifies (unmeasured)
// Question-160 correction after the first run (interim joined proof): f_d07 used `r` (`(void)r;`) with no atom on it, which is a `lifetime` error at that line for any pointer parameter (not the designed reason); it now requires owns(*r)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct R {
  int a[4];
  int *p CERASTES_REF_TO_UNINIT;
};

struct buf {
  int a[4];
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

int f_d07(struct R *r)
    CERASTES_REQUIRES(cerastes_owns(*r))
{
  (void)r;
  return 0;
}

int plain(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct R s = {{1, 2, 3, 4}, NULL};
  s.a[1] = 5;
  return s.a[1];
}

void fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->a[1] = 3;
  if (p->data != NULL) {
    *p->data = 7;
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
