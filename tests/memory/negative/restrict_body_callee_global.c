// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326), clause (ii): a restrict parameter in a function that calls a function whose contract has an atom rooted at a global (probe c1f / e1: seth() owns g, the body names no global, the contract says p == &g; the base verifies it and its twin fails, so the alias branch is live) [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
// Pre-registered from record 0064 item 19 (probe c1f), before any run: rejected
#include <cerastes.h>

int g;

void seth(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g == 2);

int f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  *p = 1;
  seth();
  return *p;
}
