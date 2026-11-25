// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_expression @ L20
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part C, Flips A): the decay of an array member (`s.a` passed as `int *`): the member's declaration and the decay
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int h(int *p) { return p != NULL; }

int f(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return h(s.a);
}
