// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/struct_value_pass_object.c
// Pins: 0044 item 5: passing *p by value reads *p, and p has no atom: permission at the call, on the first member
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int first(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int via_ptr(struct S *p)
    CERASTES_REQUIRES(p != 0)
{
  return first(*p);  // <- here
}

int use(void)
{
  struct S s = {4, 5};
  return via_ptr(&s);
}
