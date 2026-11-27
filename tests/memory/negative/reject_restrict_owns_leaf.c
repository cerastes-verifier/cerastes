// expect: rejected cerastes-unsupported_annotation @ L10
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_value_leaf.c
// Pins: 0045 item 13: a restrict parameter beside a by-value struct whose pointer leaf owns(*s.q) names: rejected at the function head [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
#include <cerastes.h>
struct P {
  int *q;
};

int f(int *restrict p, struct P s)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*s.q) && s.q == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*s.q) && r == 2)
{
  *p = 1;
  *s.q = 2;
  return *p;
}
