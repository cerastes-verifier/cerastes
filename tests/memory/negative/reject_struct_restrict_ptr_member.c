// expect: rejected cerastes-unsupported_annotation @ L16
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit S1: a restrict int * beside a struct whose pointer member reaches a struct with an int leaf [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  struct S *ps;
};

int f(int *restrict p, struct T *t)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*t))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*t))
{
  *p = 1;
  return *p;
}
