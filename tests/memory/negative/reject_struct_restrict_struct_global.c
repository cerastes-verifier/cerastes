// expect: rejected cerastes-unsupported_annotation @ L15
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit S1: a restrict int * beside a struct S * global that the contract names
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

struct S *gs;

int f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(gs))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(gs) && r >= 1)
{
  *p = 1;
  struct S *s = gs;
  if (s != NULL && &s->x == p) {
    s->x = 2;
    return *p;
  }
  return 1;
}
