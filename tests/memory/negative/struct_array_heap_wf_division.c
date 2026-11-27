// expect: error division-by-zero @ L18
// expect: error division-by-zero @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_wf.c
// Pins: 0064 item 7 (f__wf's ghosts, one per leaf and slot, shared by no two): each function guards p->a[1] != 0 and divides by another place in the same clause, so the division is by zero in the ensures, once per function. other_slot: p->a[2], the same leaf of another element; other_leaf: p->n, another leaf of the same block. A shared ghost name would prove each of them
// Pre-registered from record 0064 items 7 and 15 and the precedent negative/range_struct_wf_division.c (division-by-zero at the ensures line of each function), before any run: unmeasured for owns_block
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int other_slot(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && p->a[1] != 0)
    CERASTES_ENSURES(r, cerastes_owns_block(p) && p->a[1] != 0 && (r == 0 || r == 10 / p->a[2]))  // <- here
{
  (void)p;
  return 0;
}

int other_leaf(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && p->a[1] != 0)
    CERASTES_ENSURES(r, cerastes_owns_block(p) && p->a[1] != 0 && (r == 0 || r == 10 / p->n))  // <- here
{
  (void)p;
  return 0;
}
