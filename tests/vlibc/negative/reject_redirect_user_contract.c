// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/vlibc/user_trusted_declaration.c
// Pins: 0046 item 8 (audit B1, asm_pair.c): a user's own trusted contract is rejected too when an asm label binds it to the symbol of a function defined in the unit (in C, shared_sym's own symbol on both platforms; in C++ the mangled name differs, the label alone is refused)
// shared_sym returns 0 under its own symbol; g's contract says x + 1 for the same one.
#include <cerastes.h>
#define VLIBC_TEST_CAT2(a, b) a##b
#define VLIBC_TEST_CAT(a, b) VLIBC_TEST_CAT2(a, b)
#define VLIBC_TEST_STR2(x) #x
#define VLIBC_TEST_STR(x) VLIBC_TEST_STR2(x)
#define VLIBC_TEST_SYM(s) VLIBC_TEST_STR(VLIBC_TEST_CAT(__USER_LABEL_PREFIX__, s))

int shared_sym(int x) { (void) x; return 0; }

int g(int x) CERASTES_REQUIRES(0 <= x && x <= 100) CERASTES_ENSURES(r, r == x + 1) __asm__(VLIBC_TEST_SYM(shared_sym));

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x + 1)
{
  return g(x);
}
