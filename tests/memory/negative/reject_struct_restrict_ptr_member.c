// expect: rejected cerastes-unsupported_annotation @ L16
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit S1: a restrict int * beside a struct whose pointer member reaches a struct with an int leaf
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
