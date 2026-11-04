// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/heap_loop_local.c
// Pins: 0047 item 7 (audit B1 (b)): a return inside a loop leaves with the block allocated before the loop: the one leak row of the function follows the end label, where Silicon has merged the frame back, so it is reported at the function's closing brace
// Pre-registered from record 0047 item 7: leak at the closing brace (the row cannot tell which return leaked); re-registered after audit round 1 (record 0047 item 7): the return inside the loop
#include <cerastes.h>
#include <stdlib.h>

int early_return(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == 1 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return 1;
    }
    i += 1;
  }
  free(p);
  return 0;
}  // <- here
