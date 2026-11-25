// expect: rejected cerastes-unsupported_annotation @ L16
// expect: rejected cerastes-unsupported_annotation @ L23
// expect: rejected cerastes-unsupported_annotation @ L30
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: M4.5e1e2 Part C retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): an element read p[0].x of a pointer to structures in a requires, in an ensures under old and in a decreases. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
int f_c03(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0].x > 0) // <- here
{
    return p->x;
}

void f_c04(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p[0].x == cerastes_old(p[0].x)) // <- here
{
    (void)p;
}

int f_c05(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_DECREASES(p[0].x) // <- here
{
    return n;
}
