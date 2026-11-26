// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (amends 0052 item 2), the lifted form of negative/struct_array_reject_annotations.c's L34 (mk, a bodiless ensures atom with no requires partner on the result, unguarded) with a caller, the shape of the spike's S48 (l): the result is owned, so the caller writes and reads an element and the length without a null test; the atom on the result is accepted (0052 item 2a stays: on a formal it is rejected)
// Pre-registered from record 0064 item 15 and the answer to question 349/350 (L34 expected to lift), before any run: verifies (unmeasured: the unguarded bodiless atom on a structure with an array member has no scalar precedent)
#include <cerastes.h>

struct S {
  int a[4];
  int n;
};

struct S *mk(void)
    CERASTES_ENSURES(r, cerastes_owns(*r));

int call_mk(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S *p = mk();
  p->a[1] = 2;
  p->n = 3;
  return p->a[1] + p->n;
}
