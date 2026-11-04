// expect: error leak @ L24
// mode: c23, c++20
// twin: tests/memory/heap_loop_local.c
// Pins: 0047 item 10: a block held across a loop and never freed: one leak at the function's closing brace
#include <cerastes.h>
#include <stdlib.h>

int leaky(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
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
    *p = i;
    i += 1;
  }
  return 0;
}
