// expect: rejected cerastes-unsupported_annotation @ L14
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_global_restrict.c
// Pins: 0044 item 11 (0043 item 10): a restrict int * beside owns(g) of a struct global with an int * leaf: a pointer loaded from g is not based on p
#include <cerastes.h>

struct P {
  int a;
  int *q;
};

static struct P g;

int f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g))
{
  *p = 1;
  return g.a;
}

int main(void)
{
  int a = 0;
  return f(&a) + (g.q == 0);
}
