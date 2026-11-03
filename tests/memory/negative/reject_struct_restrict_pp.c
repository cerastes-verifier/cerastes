// expect: rejected cerastes-unsupported_annotation @ L7
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit B2: a restrict parameter beside a T ** whose *pp may be p (clang and GCC -O2 return 1, an unsound verdict otherwise)
#include <cerastes.h>

int f(int *restrict p, int **pp)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*pp) && *pp == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*pp) && r == 2)
{
  *p = 1;
  **pp = 2;
  return *p;
}
