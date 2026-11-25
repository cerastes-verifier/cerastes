// expect: rejected cerastes-unsupported_expression @ L20
// expect: rejected cerastes-unsupported_expression @ L24
// expect: rejected cerastes-unsupported_annotation @ L29
// expect: rejected cerastes-unsupported_expression @ L31
// expect: rejected cerastes-unsupported_annotation @ L35
// expect: rejected cerastes-unsupported_annotation @ L36
// mode: c23, c++20
// twin: tests/memory/struct_array_member_declared.c
// Pins: M4.5e1e2 (continuation), Part D retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): calloc of a structure with an array member (with its free), a free of a pointer to one and an owns_block atom of one (question 292). The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct S {
    int a[4];
    int n;
};
int f_d02(void)
{
    struct S *p = (struct S *)calloc(1, sizeof(struct S)); // <- here
    if (p == NULL) {
        return 0;
    }
    free(p); // <- here
    return 1;
}

void f_d03(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p)) // <- here
{
    free(p); // <- here
}

void f_d04(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p)) // <- here
    CERASTES_ENSURES(cerastes_owns_block(p)) // <- here
{
    (void)p;
}
