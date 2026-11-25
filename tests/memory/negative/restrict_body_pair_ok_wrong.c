// expect: error postcondition @ L15
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_body_pair_ok.c
// Pins: 0064 item 19 (audit round 2): the positive's cell_and_range (the pair rule's shape, n > 0 and no q == p) with r == 2 in the ensures: *p is 1 (q[0] is another cell), so the postcondition fails at the return
// Pre-registered from record 0064 item 19 as revised by audit round 2, before any run: the one error
#include <cerastes.h>

int cell_and_range(int *restrict p, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_range(q, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns_range(q, n) && r == 2)
{
  *p = 1;
  q[0] = 2;
  return *p;  // <- here
}
