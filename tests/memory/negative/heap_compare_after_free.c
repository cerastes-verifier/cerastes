// expect: error lifetime @ L19
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 6 and 0028 item 3a: using the value of p after free(p) (here in a comparison) is `lifetime`; the liveness row precedes the pointer-comparison row, so that one is not reached
// Pre-registered from record 0047 item 6 (any later use of p is lifetime): lifetime at the comparison, one error (the dead path after the failed liveness row reports no pointer-comparison and no leak of q)
#include <cerastes.h>
#include <stdlib.h>

int compare_after_free(void)
{
  int *p = (int *)malloc(sizeof(int));
  int *q = (int *)malloc(sizeof(int));
  if (p == NULL || q == NULL) {
    free(p);
    free(q);
    return -1;
  }
  free(p);
  int same = p == q;  // <- here
  free(q);
  return same;
}
