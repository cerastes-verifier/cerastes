// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 item 8: the pointer's block has ended, so the caller no longer owns *p (nor the liveness fraction the atom takes): permission at the call
#include <cerastes.h>
#include <stddef.h>

int inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p);

int dangling(void)
{
  int *p = 0;
  {
    int x = 5;
    p = &x;
  }
  return inc(p);
}
