// expect: rejected cerastes-unsupported_expression @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_read.c
// Pins: 0040 item 5 (audit B4, question 74), record 0043 item 15 (re-messaged by M4.5c1): a read of **pp in an invariant is rejected until M4.5c2
#include <cerastes.h>

void walk(int n)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp >= 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i;
    i += 1;
  }
}
