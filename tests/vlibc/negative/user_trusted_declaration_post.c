// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/vlibc/user_trusted_declaration.c
// Pins: 0011 item 5: the caller gets the declared contract and no more
// The postcondition claims x + 2; g only promises x + 1.
#include <cerastes.h>

int g(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 1);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x + 2)
{
  return g(x);
}
