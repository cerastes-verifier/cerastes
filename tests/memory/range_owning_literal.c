// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 13 (flip A4; was tests/memory/negative/reject_owning_count.c): owning(2) with a literal count on a parameter (written with CERASTES_OWNING_N, which ordinary compilers ignore, so that the file is native-ok): the function owns two elements unless the pointer is null; callers pass a two-element array and NULL
// Pre-registered from record 0060 item 13 (Part E), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING_N(2) int *p)
{
  (void)p;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[2] = {1, 2};
  f(a);
  f(NULL);
  return 0;
}
