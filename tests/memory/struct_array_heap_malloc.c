// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (heap storage, question 292; lifts the staging pin struct_array_reject_heap.c's calloc/free rows and negative/struct_array_reject_expressions.c's malloc row, Flips A (D)): malloc and calloc of a structure with an array member of scalars, every element written then read (straight line, then at a symbolic index), calloc's zero over every element (named elements and a symbolic index), free
// Pre-registered from record 0064 item 15 (Part D), before any run: verifies (unmeasured: no build of Part D yet)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int fill_sum(void)
    CERASTES_ENSURES(r, r == 10 || r == -1)
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
  int s = p->a[0] + p->a[1] + p->a[2] + p->a[3];
  free(p);
  return s;
}

int at_index(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[i] = 7;
  int v = p->a[i];
  free(p);
  return v;
}

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  CERASTES_CHECK(p->a[0] == 0 && p->a[3] == 0 && p->n == 0);
  int v = p->a[2];
  free(p);
  return v;
}

int zeroed_at(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  int v = p->a[i];
  free(p);
  return v;
}
