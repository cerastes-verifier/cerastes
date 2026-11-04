// expect: rejected cerastes-unsupported_annotation @ L7
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/ptrptr_live_inner.c
// Pins: 0045 item 13 (c1's audit-2 S1 alias shape): with live(*pp) writable the lifetime stop is gone, so the hazard is the only stop: rejected at the function head
#include <cerastes.h>

int f(int *restrict p, int **pp)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*pp) && cerastes_live(*pp) && *pp == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*pp) && cerastes_live(*pp) && r == 2)
{
  *p = 1;
  **pp = 2;
  return *p;
}
