// expect: error postcondition @ L16
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_body_owns_global.c
// Pins: 0064 item 19 (question 326): read_g with r == g in the ensures: the result is *p + g, one more, so the postcondition fails at the return
// Pre-registered from record 0064 item 19 (probe d2), before any run: the one error
#include <cerastes.h>

int g;

int read_g(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_reads(g) && r == g)
{
  *p = 1;
  return *p + g;  // <- here
}

int main(void)
{
  int a = 0;
  g = 5;
  return read_g(&a) - 6;
}
