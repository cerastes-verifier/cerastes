// expect: rejected cerastes-logical_operand_parentheses @ L14
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 and 0025 item 6 R1: the -Wall shape rules reach the `return` of a pure function named by a forall: `!(..) || a[i] >= 0 && a[i] < 9 || ..` holds an unparenthesized && inside ||, and the rejection is reported at the line of the forall that names it (the use of f in the requires), not at f's own return
// Pre-registered from edg-backend's probe build/m4.5e1c/partC/probes/shape_in_pure.c (Part C), both modes reject it the same way
#include <cerastes.h>

CERASTES_PURE bool shaped(int i, const int *a, int n)
{
    return !(0 <= i && i < n) || a[i] >= 0 && a[i] < 9 || n == 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_forall(shaped, a, n))
{
    (void)a;
    (void)n;
}
