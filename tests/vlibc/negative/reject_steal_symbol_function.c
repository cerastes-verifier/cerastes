// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 8 (re-audit): a program routine WITH a body that takes exit's own symbol by an asm label makes a call of exit reach that body, so vlibc's `ensures(false)` cannot be trusted
// The label is spelled with __USER_LABEL_PREFIX__ so that it is exit's own symbol on macOS (`_exit`) and on Linux (`exit`).
#include <stdlib.h>
#include <cerastes.h>

#define VLIBC_TEST_CAT2(a, b) a##b
#define VLIBC_TEST_CAT(a, b) VLIBC_TEST_CAT2(a, b)
#define VLIBC_TEST_STR2(x) #x
#define VLIBC_TEST_STR(x) VLIBC_TEST_STR2(x)
#define VLIBC_TEST_SYM(s) VLIBC_TEST_STR(VLIBC_TEST_CAT(__USER_LABEL_PREFIX__, s))

void my_exit(int status) __asm__(VLIBC_TEST_SYM(exit));
void my_exit(int status)
{
  (void) status;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
