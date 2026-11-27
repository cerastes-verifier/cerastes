// expect: error postcondition @ L28
// mode: c23
// twin: tests/memory/struct_init_literal_zero_member_c.c
// c-only: a lone zero that elides the braces of a structure's first member is C's universal zero initializer; in C++ item 16's other rule rejects the shape (clang++ -Wmissing-braces)
// Pins: 0064 item 16: array_member promises 1 where the zero-filled member n holds 0; the other functions are the positive's
// Pre-registered from record 0064 item 16 (question 339), before any run: postcondition at array_member's return, one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct A {
  int a[4];
  int n;
};

struct T {
  struct P p;
  int z;
};

int array_member(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct A t = {0x0};
  return t.n;
}

int array_element(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct A t = {0x0};
  return t.a[3];
}

int nested_member(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {0x0};
  return t.p.y;
}
