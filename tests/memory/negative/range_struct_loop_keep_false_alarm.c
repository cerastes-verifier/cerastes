// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/memory/range_struct_loop.c
// Pins: 0064 item 10 (listed false alarm; audit round C1, note 11; the program is correct): the loop writes p[i].x, never p[1].y, and its invariant does not say so; a store to an int member of the range reaches every int leaf of it, so the automatic conjuncts keep no value of y and the return's p[1].y == 3 is not known: postcondition at the return, once (fail closed). A written conjunct keeps it: the same loop with `p[1].y == 3` in its invariant verifies (the audit's l14, in both modes), as does the same program over a long y (the audit's f02); the precise rule is M4.8's. When it is narrowed this file is retired with a positive of the same program
// Pre-registered from record 0064 item 10 ("Listed false alarm") and the audit's f01, before any run: postcondition at the return
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int c(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1 && p[1].y == 3)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].x = 0;
    i = i + 1;
  }
  return p[1].y;
}
