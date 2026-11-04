// expect: error uninitialized @ L36
// expect: error invalid-free @ L36
// mode: c23, c++20
// twin: tests/memory/heap_loop_reassigned_free.c
// Pins: 0047 items 6 and 10's follow-up (re-audit S3, M4.5d2v): the member l->q is never written (l->q = a is missing), so freeing through it reads an uninitialized member, and the unknown value it holds may not be a block: one cause, two errors at the free, uninitialized and invalid-free; a is freed afterwards, so nothing leaks
// Pre-registered from the owner's redesign of this twin (M4.5d2v, after the first run gave uninitialized and invalid-free at the free and a leak of a): free(a) added, and the expectation is uninitialized and invalid-free at the free(l->q) line only
#include <cerastes.h>
#include <stdlib.h>

struct L {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct L *l = (struct L *)malloc(sizeof(struct L));
  if (l == NULL) {
    return -1;
  }
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    free(l);
    return -1;
  }
  int *c = a;
  *c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    c = a;
    *c = i;
    i += 1;
  }
  free(l->q);  // <- here
  free(l);
  free(a);
  return 0;
}
