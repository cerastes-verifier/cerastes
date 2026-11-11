// (M4.5e1a, plan 3.8 Flip B: the array declaration at L12 is accepted now; this line was `expect: rejected cerastes-unsupported_declaration @ L12`)
// expect: rejected cerastes-unsupported_statement @ L13
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a range-based for is rejected (and so is the array it ranges over)
// Two diagnostics: the array declaration and the statement.
#include <cerastes.h>

int total(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
{
  int s = 0;
  int a[3] = {1, 2, 3};
  for (int v : a) {
    s += v;
  }
  return s + x;
}
