// expect: rejected cerastes-unsupported_expression @ L24
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 5 (question 100; M8, over-strict): a struct ?: used as a value
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S mk(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
{
  struct S s = {a, a + 1};
  return s;
}

int f(int c)
{
  struct S s = {1, 2};
  struct S t = {3, 4};
  struct S u = {5, 6};
  t = c ? s : u;  // <- here
  return c + s.x + t.x + u.x;
}
