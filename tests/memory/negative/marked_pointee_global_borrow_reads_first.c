// expect: error uninitialized @ L21
// mode: c23, c++20
// twin: tests/memory/marked_pointee_global_borrow.c
// Pins: 0058 item 4 (audit S5): the same borrower reads gp->n before writing it: the marked global promises no value, so the read is uninitialized
// Pre-registered from record 0058 item 4 (audit S5, question 202): one uninitialized error at the read
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING CERASTES_REF_TO_UNINIT;

int touch(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(r, cerastes_owns(gp))
{
  if (gp != NULL) {
    int old = gp->n;  // <- here
    gp->n = 1;
    return old;
  }
  return 0;
}

int main(void)
{
  return 0;
}
