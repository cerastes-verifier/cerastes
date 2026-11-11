// expect: error lifetime @ L25
// mode: c23, c++20
// twin: tests/memory/array_loop_body_array.c
// Pins: 0059 item 8 and 0032 item 8: a callee keeps a[1] of the body's array: the scope exit at the body's closing brace reports lifetime
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int sum(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(s == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int a[2] = {1, 2};
    dispose(&a[1]);
    s = s + 1;
    i = i + 1;
  }
  return s;
}
