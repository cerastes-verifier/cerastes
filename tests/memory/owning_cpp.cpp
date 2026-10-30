// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0040 item 3: owning in C++ with the C++ spellings; the user's clauses read *p under the guard owning's atom frames
#include <cerastes.h>
#include <stddef.h>

void bump(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(p == nullptr || *p < 100)
    CERASTES_ENSURES(p == nullptr || *p == cerastes::old(*p) + 1)
{
  if (p != nullptr) {
    *p += 1;
  }
}

int use_bump()
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  bump(&x);
  bump(nullptr);
  return x;
}
