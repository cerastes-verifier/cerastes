// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_annotation @ L14
// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part B, Flips A): `owns(g)` and an element read `g[0]` of a global array in `requires` and `ensures`: the declaration, and the annotations that name it
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed
#include <cerastes.h>
#include <stddef.h>

int g[3];

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0] == 1)
    CERASTES_ENSURES(cerastes_owns(g) && g[0] == 1)
{
}
