// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part C, Flips A): `p->a[i]` through a pointer under `owns(*p)`: only the array member's declaration is reported (the structure is not accepted, so the access is not read)
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4)
{
  return p->a[i];
}
