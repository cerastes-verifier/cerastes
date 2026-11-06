// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (question 159): the implicit atom of an owning parameter covers: owns(*r) with the callee returning p, which the owning parameter's borrow (p == NULL || owns(*p)) already returns: the atom gives only the equality r == p, and the caller reads and writes *q through the atom it gets back
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>
#include <stddef.h>

int *id(CERASTES_OWNING int *p)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r))
{
  return p;
}

int use_id(void)
    CERASTES_ENSURES(r, r == 0 || r == 2)
{
  int x = 1;
  int *q = id(&x);
  int w = 0;
  if (q != NULL) {
    *q = 2;
    w = *q;
  }
  return w;
}
