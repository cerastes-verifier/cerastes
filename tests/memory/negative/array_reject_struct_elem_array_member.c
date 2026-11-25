// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: M4.5e1e2 Part E retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): an array of structures whose element structure has an array member (rejected until Part E, which takes depth 2). The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct T {
    int a[4];
    int n;
};
int f_b28(void)
{
    [[maybe_unused]] struct T v[2]; // <- here
    return 0;
}
