// expect: error unsequenced @ L36
// expect: error unsequenced @ L52
// expect: error unsequenced @ L68
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_seq.c
// Pins: 0064 item 11 and handoff section 3 (the sequencing rows have no other check behind them, so each is pinned): one function per row, each unsequenced at its expression, once. beside_elem: wr(p) owns the structure and p->a[1] is read in the same expression; beside_member: the same beside another member, p->n; two_calls: wr(p) twice
// Pre-registered from record 0064 item 11, before any run: one unsequenced per function, at the line of its expression (unmeasured for a heap structure)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int wr(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->a[1] == 7 && r == 0)
{
  p->a[1] = 7;
  return 0;
}

int beside_elem(void)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
  int s = wr(p) + p->a[1];  // <- here
  free(p);
  return s;
}

int beside_member(void)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
  int s = wr(p) + p->n;  // <- here
  free(p);
  return s;
}

int two_calls(void)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
  int s = wr(p) + wr(p);  // <- here
  free(p);
  return s;
}
