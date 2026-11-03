// expect: rejected cerastes-unsupported_expression @ L24
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 5 (question 100; M8): a struct store in a for step
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

int f(void)
{
  struct S t = {0, 0};
  int i = 0;
  CERASTES_DECREASES(3 - i)
  for (; i < 3; t = mk(i)) {  // <- here
    i += 1;
  }
  return t.x;
}
