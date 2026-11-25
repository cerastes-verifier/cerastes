// expect: rejected cerastes-unsupported_expression @ L19
// expect: rejected cerastes-unsupported_expression @ L27
// expect: rejected cerastes-unsupported_expression @ L35
// mode: c23, c++20
// twin: tests/memory/array_ptr_param_first.c
// Pins: M4.5e1e2 Part B retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): p[i].x written, an element read into a structure value (t = p[i]) and an element written from one (p[i] = t) through a pointer to structures. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
void f_b05(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(cerastes_owns(*p))
{
    p[i].x = 1; // <- here
}

int f_b11(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
    struct P t;
    t = p[i]; // <- here
    return t.x;
}

void f_b12(struct P *p, int i, struct P t)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(cerastes_owns(*p))
{
    p[i] = t; // <- here
}
