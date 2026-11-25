// expect: rejected cerastes-unsupported_annotation @ L14
// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/range_owning_param.c
// Pins: M4.5e1e2 (continuation), Part C retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): owning(n) on a parameter that points to structures, with its owns_range. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
void f_c06(int n, struct P *p CERASTES_OWNING_N(n)) // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n)) // <- here
{
    (void)p;
}
