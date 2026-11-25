// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part C, Flips A): `owns(p->a)` and `reads(p->a)`, the whole array member as an atom: only the array member's declaration is reported
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->a))
    CERASTES_ENSURES(cerastes_owns(p->a))
{
}
