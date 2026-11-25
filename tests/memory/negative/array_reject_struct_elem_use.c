// expect: rejected cerastes-unsupported_declaration @ L19
// expect: rejected cerastes-unsupported_expression @ L20
// expect: rejected cerastes-unsupported_declaration @ L30
// expect: rejected cerastes-unsupported_expression @ L31
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: M4.5e1e2 Part B retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): an element of an array of structures used in code: v[i].x and v[i].t.x on a local array of structures (the declaration and the use each draw a row). The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
int f_b02(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
    struct P v[3] = {{1, 2}, {3, 4}, {5, 6}}; // <- here
    return v[i].x; // <- here
}

struct Q {
    struct P t;
    int z;
};
int f_b03(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
    struct Q v[2] = {{{1, 2}, 3}, {{4, 5}, 6}}; // <- here
    return v[i].t.x; // <- here
}
