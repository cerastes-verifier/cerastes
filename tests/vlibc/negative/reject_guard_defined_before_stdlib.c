// expect: rejected cerastes-call_without_contract @ L16
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 4 (audit S1): with vlibc's include guard defined first the shim's body is skipped, so no vlibc contract is read, and the platform's own `noreturn` is never trusted
// The user declares exit as any program may (no contract); the call has none.
#define VLIBC_STDLIB_H
#include <stdlib.h>
#include <cerastes.h>

void exit(int status);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
