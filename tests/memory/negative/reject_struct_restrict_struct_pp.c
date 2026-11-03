// expect: rejected cerastes-unsupported_annotation @ L13
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10, audit S1: a restrict int * beside a struct S ** whose *pp may point at a struct whose x is *p
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

int f(int *restrict p, struct S **pp)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*pp) && r >= 1)
{
  *p = 1;
  struct S *s = *pp;
  if (s != NULL && &s->x == p) {
    s->x = 2;
    return *p;
  }
  return 1;
}
