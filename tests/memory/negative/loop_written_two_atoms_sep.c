// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/loop_written_two_atoms.c
// Pins: 0040 item 5: s may be q (when n == 5): two owns atoms on one place, the separation fails at the later atom
#include <cerastes.h>
#include <stddef.h>

void two(int *p, int *r, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*r) && 0 <= n && n <= 10)
{
  int *a = p;
  int *b = r;
  p = a;
  r = b;
  int *q = p;
  int *s = n == 5 ? p : r;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*q) && cerastes_owns(*s))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *q = i;
    *s = i;
    i += 1;
  }
}
