// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353; audit round 4's "tested so"): the whole copy of struct U { int a; int m[2][2]; int b[2]; int c; } through pointers: *p = *q under owns(*p) and reads(*q) leaves every cell of *p equal to the same cell of *q (the postcondition names all eight, so a permutation of the matrix's cells or a cell left out would fail it), and the self-copy *p = *p under owns(*p) keeps every cell of *p; negative/matrix_value_copy_ptr_wrong.c (the self-copy) and tests/long/memory/negative/matrix_value_copy_ptr_two_wrong.c (the two-pointer copy, 379 s on the box) are the twins
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): verifies (time it first: the lowering's own case of copy_ptr took 40 s)
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

void copy_ptr(struct U *p, struct U *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_reads(*q) && p->a == q->a && p->m[0][0] == q->m[0][0] && p->m[0][1] == q->m[0][1] && p->m[1][0] == q->m[1][0] && p->m[1][1] == q->m[1][1] && p->b[0] == q->b[0] && p->b[1] == q->b[1] && p->c == q->c)
{
  *p = *q;
}

void self_ptr(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a == cerastes_old(p->a) && p->m[0][0] == cerastes_old(p->m[0][0]) && p->m[0][1] == cerastes_old(p->m[0][1]) && p->m[1][0] == cerastes_old(p->m[1][0]) && p->m[1][1] == cerastes_old(p->m[1][1]) && p->b[0] == cerastes_old(p->b[0]) && p->b[1] == cerastes_old(p->b[1]) && p->c == cerastes_old(p->c))
{
  *p = *p;
}
