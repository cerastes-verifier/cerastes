// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_swap.c
// Pins: 0054 item 4 ("Pairs", entry row): two written sites that name one block at the loop's entry (q = p): the pair row fails at entry, "permission at the loop"
// Pre-registered from record 0054 item 4: permission at the invariant clause (BEST READING of "at the loop" for an entry row, as 0045 item 12's atoms in invariants; the first run may place it at the while)
#include <cerastes.h>
#include <stdlib.h>

int shared_entry(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int *q = p;
  *q = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns_block(p)) && (q == NULL || cerastes_owns_block(q)))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  free(p);
  return 0;
}
