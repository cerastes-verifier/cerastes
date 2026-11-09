// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2: owns_block(p->q) through a struct member (a place reached from a formal through a load): given by the callee after the owns(*p) that frames the load of p->q; the caller frees n.q
// Pre-registered from record 0054 item 2 (the forms "p->q", "given"): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct node {
  int *q;
  int tag;
};

void attach(struct node *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns_block(p->q) && *p->q == 7)
{
  p->q = (int *)malloc(sizeof(int));
  if (p->q == NULL) {
    abort();
  }
  *p->q = 7;
  p->tag = 1;
}

int use_attach(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct node n = {NULL, 0};
  attach(&n);
  int *q = n.q;
  int v = *q;
  free(q);
  return v;
}
