// expect: rejected cerastes-unsupported_annotation @ L12
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10: a restrict int * beside a struct pointer with no atom of its own: the struct may contain the int's cell
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(int *restrict q, struct S *p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*q))
{
  (void)p;
}
