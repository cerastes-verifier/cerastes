// expect: error invalid-free @ L27
// (one row: the token row fails and ends the path, so the offset row is not reached; owner question 143, M4.5d2v)
// mode: c23, c++20
// twin: tests/memory/heap_loop_reassigned_free.c
// Pins: 0047 items 6 and 10's follow-up (re-audit S3, M4.5d2v): a pointer the loop reassigns to a local's address (c = &x) and then freed after the loop: the free is invalid-free (a local is no allocation), reported at the free in user terms, and never an internal check (exit 3)
// Pre-registered from the owner's redesign of this twin (M4.5d2v, after the first run reported a genuine invalid-free at free(h): with n == 0 c is h): h is freed first and then c, so only free(c) fails: one invalid-free error at it, the token row, which ends the path (the first run showed the offset row is not reached)
#include <cerastes.h>
#include <stdlib.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *h = (int *)malloc(sizeof(int));
  if (h == NULL) {
    return -1;
  }
  int *c = h;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    c = &x;
    i += 1;
  }
  free(h);
  free(c);  // <- here
  return x;
}
