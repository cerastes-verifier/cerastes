// expect: rejected cerastes-unsupported_declaration @ L15
// expect: rejected cerastes-unsupported_declaration @ L23
// expect: rejected cerastes-unsupported_expression @ L24
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: M4.5e1e2 (continuation), Part C retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): a global array of structures and a static local array of structures with an element read. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
struct P g[3]; // <- here
int f_c08(void)
{
    return 0;
}

int f_c10(void)
{
    static struct P t[2]; // <- here
    return t[0].x; // <- here
}
