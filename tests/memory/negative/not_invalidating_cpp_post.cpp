// expect: error postcondition @ L28
// mode: c++20
// twin: tests/memory/not_invalidating_cpp.cpp
// Pins: 0058 item 2: the C++ spellings with a caller whose ensures promises r == 4 where twice(&x) gives 3 (1 + 2): the marked contract is used as written, one postcondition error at the return of use
// Pre-registered from record 0058 item 2: one postcondition error at the return of use
#include <cerastes.h>

void bump(int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes::owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes::owns(*p) && *p == cerastes::old(*p) + 1)
{
  *p += 1;
}

int twice(CERASTES_NOT_INVALIDATING int *p)
    CERASTES_REQUIRES(cerastes::owns(*p) && *p < 98)
    CERASTES_ENSURES(r, cerastes::owns(*p) && r == *p && r == cerastes::old(*p) + 2)
{
  bump(p);
  bump(p);
  return *p;
}

int use()
    CERASTES_ENSURES(r, r == 4)
{
  int x = 1;
  return twice(&x);  // <- here
}
