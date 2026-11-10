// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0058 item 2: the marker with the C++ spellings of the contract atoms, a declaration and a definition both marked, in the trailing and the leading position; the twin of the pure and predicate pins
// Pre-registered from record 0058 item 2: verifies in c++20
#include <cerastes.h>

void bump(int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes::owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes::owns(*p) && *p == cerastes::old(*p) + 1);

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
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  return twice(&x);
}
