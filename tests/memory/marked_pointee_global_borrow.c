// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): a function borrows an owning and marked global pointer to a structure with an owning field (owns(gp) in and out) and writes a member without reading it; no caller is needed: the borrower body is what is checked
// Pre-registered from record 0058 item 4 (audit S5, question 202): verifies in both modes (edg-backend's s5_global.c shape)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING CERASTES_REF_TO_UNINIT;

void touch(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(cerastes_owns(gp))
{
  if (gp != NULL) {
    gp->n = 1;
  }
}

int main(void)
{
  return 0;
}
