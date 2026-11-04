// expect: error leak @ L36
// mode: c23, c++20
// twin: tests/memory/heap_loop_nested_return.c
// Pins: 0047 items 7, 10: the return inside the nested loops no longer frees the block: the one leak row of the function, at its closing brace
// Pre-registered from the loops branch's positive (record 0047 item 10), written after it: the expectation was verified once on the single file before it was written
#include <cerastes.h>
#include <stdlib.h>

int nested(int n)
    CERASTES_REQUIRES(0 <= n && n <= 20)
    CERASTES_ENSURES(r, r == 0 || r == -1 || r == 1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n)
    CERASTES_DECREASES(n - j)
    while (j < n) {
      *p = i + j;
      j += 1;
    }
    if (i == 3) {
      return 1;
    }
    i += 1;
  }
  free(p);
  return 0;
}  // <- here
