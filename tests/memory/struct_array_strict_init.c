// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0064 item 15 (294) under --strict-init (0042 item 8, 0052 item 4): a local that receives malloc's result is marked ref_to_uninit and its structure's every element is written before it is read; calloc's result needs no marker; an initialized structure local needs none
// Pre-registered from record 0064 item 15 and the precedent strict_marked_local_alloc.c, before any run: verifies under --strict-init in both modes (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int heap_fill(void)
    CERASTES_ENSURES(r, r == 10 || r == -1)
{
  CERASTES_REF_TO_UNINIT struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
  int s = p->a[0] + p->a[1] + p->a[2] + p->a[3];
  free(p);
  return s;
}

int via_calloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *q = (struct S *)calloc(1, sizeof(struct S));
  if (q == NULL) {
    return -1;
  }
  int v = q->a[2];
  free(q);
  return v;
}

int local_init(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return s.n;
}
