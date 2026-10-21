// expect: rejected cerastes-call_without_contract @ L13
// mode: c23, c++20
// twin: tests/scalar/noreturn_libc.c
// Pins: 0015 item 5: only abort, exit and _Exit are in the trusted table; quick_exit has no contract
// (tag observed with the pre-0015 binary: a function with no body and no contract)
#include <stdlib.h>
#include <cerastes.h>

int leave(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    quick_exit(1);
  }
  return x;
}
