// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (amends 0052 item 2): a fresh result with an owns atom that has no requires partner, of a structure with an array member: guarded (r == NULL || owns(*r)); the caller tests it and writes and reads an element. The unguarded bodiless form of the pin (mk, ensures(r, owns(*r)) and call_mk) has no scalar precedent and is left out: report 1, question 5
// Pre-registered from record 0064 item 15 and the precedent result_bodiless.c, before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct S *mk(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

int use_fresh(void)
    CERASTES_ENSURES(r, r == 0 || r == 2)
{
  struct S *p = mk();
  int w = 0;
  if (p != NULL) {
    p->a[1] = 2;
    w = p->a[1];
  }
  return w;
}
