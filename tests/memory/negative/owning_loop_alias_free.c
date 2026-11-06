// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owning_loop_alias.c
// Pins: 0052 item 3 Loops, shape L6: the loop frees the owner's block and then writes through the alias a, which designated it: the alias's place is the owner's own block, gone with the free: permission at the write (the function ends in abort() after the loop: with a normal return the model's exit state, in which the alias may be the owner's block or a block of its own, adds a second error, a leak or an invalid-free, that is no part of this twin: a program that frees the owner in the loop and uses the alias after it is wrong either way)
// Pre-registered from record 0052 item 3 Loops (il-analyst's probes build/m4.5d2b/loop/p): permission at the write through the alias
#include <cerastes.h>
#include <stdlib.h>

int through_alias(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    free(v);
    v = NULL;
    *a = k;  // <- here
  }
  abort();
}
