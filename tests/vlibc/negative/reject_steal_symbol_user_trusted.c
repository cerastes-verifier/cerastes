// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23, c++20
// twin: tests/vlibc/user_trusted_declaration.c
// Pins: 0046 item 8 (re-audit): a user's own bodiless declaration with a contract is untrusted as well when a routine defined in the unit takes its symbol by an asm label
// impl returns 0 under g's own symbol; g's contract says x + 1.
#include <cerastes.h>

#define VLIBC_TEST_CAT2(a, b) a##b
#define VLIBC_TEST_CAT(a, b) VLIBC_TEST_CAT2(a, b)
#define VLIBC_TEST_STR2(x) #x
#define VLIBC_TEST_STR(x) VLIBC_TEST_STR2(x)
#define VLIBC_TEST_SYM(s) VLIBC_TEST_STR(VLIBC_TEST_CAT(__USER_LABEL_PREFIX__, s))

int g(int x) CERASTES_REQUIRES(0 <= x && x <= 100) CERASTES_ENSURES(r, r == x + 1);

int impl(int x) __asm__(VLIBC_TEST_SYM(g));
int impl(int x)
{
  (void) x;
  return 0;
}

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x + 1)
{
  return g(x);
}
