// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_expression @ L19
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part C, Flips A): a structure local's array member read by index (`s.a[i]`): the member itself is rejected, and so is the access
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return s.a[i];
}
