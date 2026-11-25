// expect: rejected cerastes-unsupported_declaration @ L11
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part B, Flips A): a `static` local array that the function reads: the declaration is rejected (array_reject_static_local.c has an unused one)
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  static int t[2] = {5};
  return t[0];
}
