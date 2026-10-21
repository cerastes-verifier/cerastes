// expect: rejected cerastes-call_without_contract @ L12
// c-only: __builtin_unreachable is a GNU builtin
// twin: tests/scalar/noreturn_libc.c
// Pins: 0015 item 1: __builtin_unreachable is not in the trusted table
// (tag observed with the pre-0015 binary: a function with no body and no contract)
#include <cerastes.h>

int leave(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    __builtin_unreachable();
  }
  return x;
}
