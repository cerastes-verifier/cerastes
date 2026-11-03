// expect: rejected cerastes-unsupported_annotation @ L7
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 13: a restrict parameter beside owns(**pp): rejected by restrict_hazard from pp's type (a T** can hold a pointer to p's object)
#include <cerastes.h>

int f(int *restrict p, int **pp)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*pp) && cerastes_owns(**pp) && *pp != p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*pp) && cerastes_owns(**pp) && r == 2)
{
  *p = 1;
  **pp = 2;
  return *p;
}
