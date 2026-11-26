// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D's loop column; a heap block's array part as a place of a loop): a calloc'ed structure with an array member filled by a loop and freed: the permissions of the array part are the loop's (kept at the entry amounts, the flags kept), so the stores in the body and the read after the loop verify
// Pre-registered from the plan's Part D table (section 7: `loop_places` and `loop` with an array leaf's lines) and record 0064 item 15 (heap storage), read with record 0047 item 10 (a heap block is a place of a loop, with a back-edge row), and il-analyst's probe d1probe/a3 (case e1e2d_loop_heap_fill), before any run of this file: verifies in both modes; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements Corrected at the second interim run (question 160): `(void)v;` drew C++20's front-end warning #550 (`v` set but never used) at L29, which a test may not print, so the use is `return v - v;`; the program's rows are unchanged
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
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
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
  int v = p->a[3];
  free(p);
  return v - v;
}
