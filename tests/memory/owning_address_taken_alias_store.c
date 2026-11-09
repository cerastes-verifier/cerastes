// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 ("Stores"): a store through an alias of an owning address-taken local (*pp = ... with pp == &v) gets the same rows as a store by name: the old block was freed, so nothing is dropped, and v owns the new block, which is freed
// Pre-registered from record 0054 item 6 (S39 f1): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int alias_store(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  int **pp = &v;
  free(*pp);
  *pp = (int *)malloc(sizeof(int));
  free(v);
  return 0;
}
