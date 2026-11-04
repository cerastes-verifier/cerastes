// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/vlibc/user_trusted_declaration.c
// Pins: 0046 item 8 (audit B1, asm_pair.c): a user's own trusted contract is rejected too when an asm label binds it to a symbol that has another body in the unit
// impl returns 0 under the symbol shared_sym; g's contract says x + 1 for the same symbol.
#include <cerastes.h>

int impl(int x) __asm__("shared_sym");
int impl(int x)
{
  (void) x;
  return 0;
}

int g(int x) CERASTES_REQUIRES(0 <= x && x <= 100) CERASTES_ENSURES(r, r == x + 1) __asm__("shared_sym");

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x + 1)
{
  return g(x);
}
