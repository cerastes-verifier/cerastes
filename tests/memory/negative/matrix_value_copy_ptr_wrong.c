// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/matrix_value_copy_ptr.c
// Pins: 0064 item 12 (question 353; question 285's rule: the short twin of the long two-pointer one): the self-copy *p = *p keeps every cell, so a postcondition that the cell p->m[1][0] is one more than it was fails at the closing brace; its right-value sibling is self_ptr in tests/memory/matrix_value_copy_ptr.c (the same function with the cell equal to its old value)
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): the one postcondition error at the closing brace
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

void self_ptr_wrong(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->m[1][0] < 1000)
    CERASTES_ENSURES(cerastes_owns(*p) && p->m[1][0] == cerastes_old(p->m[1][0]) + 1)
{
  *p = *p;
}  // <- here
