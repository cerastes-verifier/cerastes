// expect: rejected cerastes-unsupported_annotation @ L12
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_local.c
// Pins: 0043 item 10: restrict on a struct pointer parameter is rejected (M8)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(struct S *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
}
