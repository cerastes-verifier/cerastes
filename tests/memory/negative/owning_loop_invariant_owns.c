// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/owning_loop_invariant_reads.c
// Pins: 0052 item 3 Loops, shape L7: a written owns(*v) of a carried owning local in the invariant: the loop already owns the owner's block through the owning local, so the atom is refused (permission at the atom: drop it)
// Pre-registered from record 0052 item 3 Loops (il-analyst's probes build/m4.5d2b/loop/p): permission at the atom
#include <cerastes.h>
#include <stdlib.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_INVARIANT(cerastes_owns(*v))  // <- here
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    *v = k;
  }
  free(v);
  return 0;
}
