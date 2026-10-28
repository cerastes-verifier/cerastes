// expect: error postcondition @ L26
// mode: c23, c++20
// twin: tests/memory/scope_exits.c
// Pins: 0032 item 8: z is one too many, so the return from the innermost block breaks the postcondition
#include <cerastes.h>

int scopes(int a)
    CERASTES_REQUIRES(0 < a && a < 1000)
    CERASTES_ENSURES(r, r == a + 3)
{
  int x = a;
  int *p = &x;
  {
    int w = 1;
    int *pw = &w;
    *pw = 3;
    x += *pw;
  }
  if (a > 500) {
    int y = *p;
    int *q = &y;
    {
      int z = *q + 1;
      int *s = &z;
      if (*s > 0) {
        return *s;
      }
    }
    return y;
  }
  return x;
}
