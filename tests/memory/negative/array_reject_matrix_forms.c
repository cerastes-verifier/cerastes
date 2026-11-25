// expect: rejected cerastes-unsupported_declaration @ L16
// expect: rejected cerastes-unsupported_expression @ L17
// expect: rejected cerastes-unsupported_declaration @ L20
// expect: rejected cerastes-unsupported_declaration @ L26
// expect: rejected cerastes-unsupported_type @ L26
// expect: rejected cerastes-unsupported_expression @ L40
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: M4.5e1e2 (continuation), Part E retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): a matrix used in code (m[1][2]), a global matrix, an int a[2][3] parameter and p[i].a[j] through a pointer to structures with an array member. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
int f_e02(void)
{
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}}; // <- here
    return m[1][2]; // <- here
}

int gm[2][3]; // <- here
int f_e09(void)
{
    return 0;
}

int f_e14(int a[2][3]) // <- here
{
    (void)a;
    return 0;
}

struct T {
    int a[4];
    int n;
};
int f_e25(struct T *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0 && j == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
    return p[i].a[j]; // <- here
}
