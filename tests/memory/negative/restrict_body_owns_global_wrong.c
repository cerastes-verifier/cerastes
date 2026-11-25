// expect: error postcondition @ L17
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_body_owns_global.c
// Pins: 0064 item 19 (question 326): the same program with r == 2 in the ensures: *p is 1 (g is another object), so the postcondition fails at the return
// Pre-registered from record 0064 item 19 (probe d1), before any run: the one error
#include <cerastes.h>

int g;

int own_g(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && g == 2 && r == 2)
{
  *p = 1;
  g = 2;
  return *p;  // <- here
}

int main(void)
{
  int a = 0;
  return own_g(&a) - 1;
}
