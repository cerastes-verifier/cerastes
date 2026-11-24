// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c++20
// twin: tests/memory/pure_ensures_after_write.c
// Pins: 0062 items 1 and 4 (audit round 1 B2): an application as the index inside the operand of cerastes::old is rejected (M4.5e2)
// Pre-registered from record 0062 (Part C), before the code: rejected. the tag is the audit's and the record's (unsupported_annotation, M4.5e2); today the program gives call_in_annotation at the application (audit1-scratch/old_nested*.cpp), which Part C's lifting of a call in an ensures must replace
#include <cerastes.h>

CERASTES_PURE int gp(int x)
{
  return x > 0;
}

int pick3(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2))
    CERASTES_ENSURES(r, cerastes_owns_range(a, 2) && r == cerastes::old(a[gp(n)]))
{
  return a[0];
}
