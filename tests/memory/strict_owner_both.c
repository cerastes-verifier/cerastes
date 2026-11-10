// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// Pins: 0058 item 3: both strict options on one verify-options line: --strict-init asks for the marker on a local without an initializer and rejects malloc (calloc is unmarked, 0047 item 8), --strict-owner asks for an owner for calloc's block; the program satisfies both
// Pre-registered from record 0058 item 3: verifies in both modes under --strict-init --strict-owner
#include <cerastes.h>
#include <stdlib.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_UNINIT int x;
  init(&x);
  CERASTES_OWNING int *v = (int *)calloc(1, sizeof(int));
  if (v == NULL) {
    return -1;
  }
  int y = *v;
  free(v);
  return x + y;
}
