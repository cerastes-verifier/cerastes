// expect: rejected cerastes-unsupported_annotation_expression @ L15
// mode: c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 4: cerastes::old over an application is rejected (an application counts as a read; pinned in C++, where it compiles; M4.5e2), at the application
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int dbl(int x)
{
  return x + x;
}

int use(int y)
    CERASTES_REQUIRES(-100 < y && y < 100)
    CERASTES_ENSURES(r, r == cerastes::old(dbl(y)))
{
  return y;
}
