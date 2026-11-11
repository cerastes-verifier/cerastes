// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0058 item 4 (under --strict-init): a store p->data = malloc(...) into a marked field is accepted: the marked field is the receiver --strict-init asks for (reject_strict_init_malloc_member.c, the same store into an unmarked field, stays rejected)
// Pre-registered from record 0058 item 4: verifies in both modes under --strict-init
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

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
  CERASTES_OWNING struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  buf_destroy(b);
  return 0;
}
