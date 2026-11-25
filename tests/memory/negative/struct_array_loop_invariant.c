// expect: error invariant-preserved @ L20
// mode: c23, c++20
// twin: tests/memory/struct_array_loops.c
// Pins: 0063 item 10: the body writes s.n = 1 while the invariant says s.n == 0: not preserved, reported at the loop (the clauses stand on the while's line)
// Pre-registered from record 0063 item 10 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 4)
    CERASTES_ENSURES(r, r == n)
{
  struct S s = {{0}, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s.n == 0) CERASTES_DECREASES(n - i) while (i < n) {  // <- here
    s.a[i] = i;
    s.n = 1;
    i = i + 1;
  }
  return i + s.a[0] * 0;
}
