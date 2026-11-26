// expect: error uninitialized @ L29
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_heap_fill.c
// Pins: 0064 item 15, "A heap block's array part in a loop" (a listed false alarm, fail closed; Part F's written quantified invariants lift it): after a fill loop over a malloc'ed block the flags of the array part are the loop entry's (the automatic conjuncts keep the flags that were set at the entry and say nothing of the ones the body sets), so the read of an element after the loop is `uninitialized` for a correct program, as for a local array (negative/array_loop_fill_then_read_false_alarm.c); the same program over calloc verifies (the twin)
// Pre-registered from edg-backend report 6, (d) (measured: malloc, a fill loop, a read of p->a[3]: `uninitialized` at the read line, both modes; build/m4.5e1e2c/t354/d/fill_read.c) and il-analyst's report 3, limit 4, before any run of this file as a test Corrected at the second interim run (question 160): `(void)v;` drew C++20's front-end warning #550 (`v` set but never used) at L29, which a test may not print, so the use is `return v - v;`; the program's rows are unchanged
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int fill_and_read(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = i;
    i = i + 1;
  }
  int v = p->a[3];  // <- here
  free(p);
  return v - v;
}
