// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_owner_merged.c
// Pins: 0054 item 4 ("Pairs": the same rows stand between a written site and each owner carrier when the site names a different variable): the written site is on w, a non-owning alias of the owner v's block, so both name one block and the pair row fails at entry
// Pre-registered from record 0054 item 4: permission at the invariant clause (entry row; BEST READING of the line, see owns_block_invariant_shared_entry.c)
#include <cerastes.h>
#include <stdlib.h>

void touch(void) {}

int merged(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 0;
  int *w = v;
  *w = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (w == NULL || cerastes_owns_block(w)))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    touch();
    *v = i;
    i += 1;
  }
  free(v);
  return 0;
}
