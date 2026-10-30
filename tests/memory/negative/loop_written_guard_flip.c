// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/loop_written_atom_param.c
// Pins: 0040 item 5 (audit N5): the guard q == NULL || owns(*q) is false at entry, so the loop holds nothing of p's place; q = p makes it true and the back edge cannot show the permission (live(p) is written so the copy itself is fine)
#include <cerastes.h>
#include <stddef.h>

int flip(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, cerastes_owns(*p) && (r == 0 || r == n))
{
  int *a = p;
  p = a;
  int *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_live(p) && (q == NULL || cerastes_owns(*q)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    i += 1;
  }
  return q == NULL ? 0 : n;
}
