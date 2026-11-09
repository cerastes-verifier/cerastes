// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 4 ("Merging"): a written owns_block site on an owning loop variable's own value merges with that variable's carrier (one carrier per block, not guarded against earlier carriers); the loop has a call, so v is a full carrier either way
// Pre-registered from record 0054 item 4: verifies in both modes
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (v == NULL || cerastes_owns_block(v)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    touch();
    *v = i;
    i += 1;
  }
  free(v);
  return 0;
}
