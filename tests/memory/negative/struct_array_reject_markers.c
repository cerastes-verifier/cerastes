// expect: rejected cerastes-unsupported_annotation @ L15
// expect: rejected cerastes-unsupported_annotation @ L22
// expect: rejected cerastes-unsupported_annotation @ L34
// mode: c23, c++20
// twin: tests/memory/struct_array_owns_array.c
// Pins: M4.5e1e2 Part D retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): must_init on a pointer to a structure with an array member (no row existed), an owning result of such a pointer and a ref_to_uninit local. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct S {
    int a[4];
    int n;
};
void f_d09(struct S *p CERASTES_MUST_INIT) // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
    (void)p;
}

CERASTES_OWNING struct S *f_d11(void); // <- here

void g_d13(struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*q))
{
    (void)q;
}
void f_d13(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
    CERASTES_REF_TO_UNINIT struct S *q = p; // <- here
    g_d13(q);
}
