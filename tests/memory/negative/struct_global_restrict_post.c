// expect: error postcondition @ L19
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_global_restrict.c
// Pins: 0044 item 11: the same program with *p == 2 in the ensures: postcondition at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g;

int f(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 2 && r == g.x)
{
  *p = 1;
  return g.x;  // <- here
}

int main(void)
{
  int a = 0;
  return f(&a);
}
