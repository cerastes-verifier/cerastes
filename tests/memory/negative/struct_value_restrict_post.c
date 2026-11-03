// expect: error postcondition @ L17
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_value_restrict.c
// Pins: 0044 item 11: the same program with *p == s.y in the ensures: postcondition at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(int *restrict p, struct S s)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == s.y && r == s.y)
{
  *p = s.x;
  return s.y;  // <- here
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = 0;
  struct S s = {1, 2};
  return f(&a, s);
}
