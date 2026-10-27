// expect: error shift @ L15
// mode: c23, c++20
// twin: tests/scalar/static_assert_nested.c
// Pins: 0025 item 5: an assertion inside a loop body is checked as written: 1 << 31 does not fit int (EDG, clang and gcc accept it in both modes; 1 << 40 is refused by C++ mode itself)
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    static_assert((1 << 31) != 0, "the shift does not fit int");
    i++;
  }
  return i;
}
