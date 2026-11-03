// expect: verified
// expect: native-ok
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// Pins: 0044 item 11: a restrict int * beside owns(g) of a struct global whose members are all scalars is accepted
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g;

int f(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && r == g.x)
{
  *p = 1;
  return g.x;
}

int main(void)
{
  int a = 0;
  return f(&a);
}
