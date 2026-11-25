// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part C (a structure global; cut 6 would re-point it), Flips A): a structure global with an array member and an initializer, read in `main`: the member's declaration and the global's
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct S g = {{1, 2, 3, 4}, 5};

int main(void)
{
  return g.n;
}
