// expect: golden
// expect: native-ok
// Pins: decision 0006 item 6 (must match): the contract written on the header
// declaration, repeated identically on a bare-style redeclaration and on the
// definition with RENAMED parameters (compared by position), and a redeclaration
// with no contract. All declarations of the function show the one contract.
#include <cerastes.h>

int bump(int x) CERASTES_REQUIRES(x > 0 && x < 100) CERASTES_ENSURES(r, r > x);
int bump(int y);
int bump(int z) CERASTES_REQUIRES(z > 0 && z < 100) CERASTES_ENSURES(r, r > z)
{
  return z + 1;
}
