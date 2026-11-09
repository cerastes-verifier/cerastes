// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/owning_loop_alias_shared_at_entry.c
// Pins: 0054 item 3 (re-audit S2): the body writes through the alias a after free(v): the block is gone, and a is not an owner (the one row that fails; the program ends the path there)
// Pre-registered from record 0054 item 3 (as owning_loop_alias_free.c, 0052 item 3 Loops shape L6): permission at the write through the alias
#include <cerastes.h>
#include <stdlib.h>

int alias_shared(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n && v != NULL)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    a = v;
    *a = k;
    free(v);
    *a = 1;  // <- here
    v = (int *)malloc(sizeof(int));
    if (v == NULL) {
      abort();
    }
    *v = 0;
  }
  free(v);
  return 0;
}
