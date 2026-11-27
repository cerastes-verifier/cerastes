// expect: error check @ L37
// expect: error check @ L55
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_written_neighbours.c
// Pins: 0064 item 15 (Part D, step B): the twin of the two accepted neighbours: the same two functions, each with a check(0) after its loop that must fail at its own line (the accepted atoms verify and do not hide the check)
// Pre-registered from the auditor's probe audit-D2/q03_348_cell.c (check(0) fails at its line), before any run of this file as a test: one check error per function
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct E {
  int x;
  int y;
};

struct SA {
  struct E v[2];
  int n;
};

void cell(struct SA *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->n))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p->n = i;
    i = i + 1;
  }
  CERASTES_CHECK(0);  // <- here
}

void guarded(struct S *p, int n)
    CERASTES_REQUIRES((p == NULL || cerastes_owns(*p)) && 0 <= n && n <= 100)
    CERASTES_ENSURES(p == NULL || cerastes_owns(*p))
{
  struct S *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns(*p)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (p != NULL) {
      p->a[1] = i;
    }
    i += 1;
  }
  CERASTES_CHECK(0);  // <- here
}
