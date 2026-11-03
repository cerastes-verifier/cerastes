// expect: rejected cerastes-unsupported_expression @ L20
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 5 (audit S4; M8): a return source with a call in its base: return *g1(p) is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S *g1(struct S *p)
{
  return p;
}

struct S f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  return *g1(p);  // <- here
}
