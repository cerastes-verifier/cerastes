// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (293; 0040 item 3: owning on a parameter is the borrow p == NULL || owns(*p), here of a structure with an array member; the pin struct_array_reject_annotations.c's owning_param is bump): the leading and the trailing position, the body tests p, the callers pass &s and NULL
// Pre-registered from record 0064 item 15 and the precedent owning_param.c, before any run: verifies (unmeasured: the borrow's array leaf lines under a guard have no precedent)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void bump(CERASTES_OWNING struct S *p)
{
  if (p != NULL) {
    p->a[0] = 1;
    p->n = 2;
  }
}

void bump_trailing(struct S *p CERASTES_OWNING)
{
  if (p != NULL) {
    p->a[3] = 4;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {{1, 2, 3, 4}, 5};
  bump(&s);
  bump_trailing(&s);
  bump(NULL);
  s.n = 3;
  return s.n;
}
