// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part E, Flips A): a by-value parameter of a structure with an array member: the member's declaration and the parameter's
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(struct S s)
{
  return s.n;
}
