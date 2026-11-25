// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part E, Flips A): the whole-structure copy `t = s` of a structure with an array member: only the member's declaration is reported
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t;
  t = s;
  return t.n;
}
