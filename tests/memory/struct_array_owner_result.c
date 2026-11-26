// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (293; 0047 item 5: owning on a result is r == NULL || owns_block(r); the pin struct_array_reject_markers.c's f_d11 is make_bodiless): an owning result of a structure with an array member, from calloc and bodiless, received by an owning local that writes an element and frees
// Pre-registered from record 0064 item 15 and the precedents heap_owning_result.c and owning_local_free.c, before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING struct S *make(void)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  return p;
}

CERASTES_OWNING struct S *make_bodiless(void);

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct S *p = make();
  if (p == NULL) {
    return -1;
  }
  p->a[1] = 9;
  int v = p->a[1];
  free(p);
  return v - 9;
}

int use_bodiless(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct S *q = make_bodiless();
  if (q == NULL) {
    return -1;
  }
  q->n = 1;
  free(q);
  return 0;
}
