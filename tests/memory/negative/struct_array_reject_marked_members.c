// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_annotation @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_member_declared.c
// Pins: M4.5e1e2 Part D retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): a ref_to_uninit member beside an array member (question 293's marked half: no row existed) and an owning member that points to a structure with an array member. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct R {
    int a[4]; // <- here
    int *p CERASTES_REF_TO_UNINIT;
};
int f_d07(struct R *r)
{
    (void)r;
    return 0;
}

struct S {
    int a[4];
    int n;
};
struct R_d12 {
    CERASTES_OWNING struct S *q; // <- here
    int z;
};
int f_d12(struct R_d12 *r)
{
    (void)r;
    return 0;
}
