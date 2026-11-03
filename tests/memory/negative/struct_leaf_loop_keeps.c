// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/struct_leaf_loop.c
// Pins: 0045 item 10: a callee in the body keeps *s.q: the back-edge row of the automatic place fails (permission)
#include <cerastes.h>
#include <stddef.h>
void keeper(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));

struct P {
  int *q;
};

void bump_n(struct P s, int n)
    CERASTES_REQUIRES(cerastes_owns(*s.q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*s.q))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    keeper(s.q);
    i += 1;
  }  // <- here
}
