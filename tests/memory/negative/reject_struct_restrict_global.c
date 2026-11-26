// expect: rejected cerastes-unsupported_annotation @ L9
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit B2: a restrict parameter beside a pointer global gp == p [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
#include <cerastes.h>

int *gp;

int f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(gp) && gp == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(gp) && r == 2)
{
  *p = 1;
  *gp = 2;
  return *p;
}
