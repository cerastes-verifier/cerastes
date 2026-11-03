// expect: rejected cerastes-unsupported_annotation @ L9
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/global_pointer_target.c
// Pins: 0045 item 13: a restrict parameter beside a pointer global whose target owns(*gp) names: rejected at the function head
#include <cerastes.h>

int *gp;

int f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(gp) && cerastes_owns(*gp) && gp == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp) && r == 2)
{
  *p = 1;
  *gp = 2;
  return *p;
}
