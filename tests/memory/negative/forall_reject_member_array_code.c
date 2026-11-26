// expect: rejected cerastes-unsupported_annotation @ L23
// expect: rejected cerastes-unsupported_annotation @ L35
// expect: rejected cerastes-forall_function @ L52
// (the owns_range over structures at L52 drew unsupported_annotation here until Part C1 lifted it, question 340; its row is retired, the line stays)
// mode: c23, c++20
// twin: tests/memory/struct_array_owns_array.c
// Pins: M4.5e1e2 (continuation), Part F retires or re-expects this staging pin (plan section 10, Flips A; R2 of Step 0): cerastes_forall over a member array in a check and in a loop invariant, and over a pointer to structures (p[i].x under the binder). The rows are what the frozen base build (f07d80a) printed in both modes [Part C1 (question 340): the unsupported_annotation row of L52, owns_range over structures, is retired by the lift; forall_function @ L52 stays for Part F]
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct S {
    int a[4];
    int n;
};
CERASTES_PURE bool pos_f02(int i, const int *p, int n)
{
    return !(0 <= i && i < n) || p[i] > 0;
}
int f_f02(void)
{
    struct S s = {{1, 2, 3, 4}, 5};
    CERASTES_CHECK(cerastes_forall(pos_f02, s.a, 4)); // <- here
    return s.n;
}

CERASTES_PURE bool pos_f03(int i, const int *p, int n)
{
    return !(0 <= i && i < n) || p[i] > 0;
}
int f_f03(void)
{
    struct S s = {{1, 2, 3, 4}, 5};
    int i = 0;
    CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(pos_f03, s.a, 4)) // <- here
    CERASTES_DECREASES(3 - i)
    while (i < 3) {
        i = i + 1;
    }
    return s.n;
}

struct P {
    int x;
    int y;
};
CERASTES_PURE bool px_f07(int i, const struct P *p, int n)
{
    return !(0 <= i && i < n) || p[i].x > 0;
}
void f_f07(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_forall(px_f07, p, n)) // <- here
{
    (void)p;
}
