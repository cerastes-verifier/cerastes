// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/heap_loop_local.c
// Pins: 0047 item 10: the body frees the block the loop holds, so the back edge no longer has its token: permission at the body's closing brace (0038 item 4's shape); the free after the loop is fine, the loop's places being assumed held there
// Pre-registered from record 0047 item 10 (the back-edge row perm(tok) >= kt): permission at the closing brace of the loop body
#include <cerastes.h>
#include <stdlib.h>

int loop_holds_block(int n)
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
    free(p);
    i += 1;
  }  // <- here
  free(p);
  return 0;
}
