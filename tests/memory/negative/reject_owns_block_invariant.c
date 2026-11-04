// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/heap_borrow_then_free.c
// Pins: 0047 item 4: the `owns_block` forms of item 4 that stay rejected: a written owns_block in a loop invariant (d2b)
// Pre-registered from record 0047 item 4: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_block(p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns_block(p))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
