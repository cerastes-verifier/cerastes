// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0040 item 5 (replaces reject_atom_in_loop_invariant_cpp.cpp): owns(*p) written in an invariant on an automatic place (p is never assigned): the atom takes nothing twice
#include <cerastes.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes::owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes::owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes::owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
