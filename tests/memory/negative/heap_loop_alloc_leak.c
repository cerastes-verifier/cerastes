// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/heap_loop_local.c
// Pins: 0047 items 7, 10 (audit B1 (a)): an iteration ends owning a block that the loop does not carry to the next one (the invariant names no token place for it): leak at the loop body's closing brace, before Viper exhales the invariant and drops the block
// Pre-registered from record 0047 item 7: leak at the closing brace of the loop body, one error (the function's own end row finds nothing held); re-registered after audit round 1 (record 0047 item 7): a row at every loop back edge
#include <cerastes.h>
#include <stdlib.h>

int loop_allocates(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *p = (int *)malloc(sizeof(int));
    if (p != NULL) {
      *p = i;
    }
    i += 1;
  }  // <- here
  return 0;
}
