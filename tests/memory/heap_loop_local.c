// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10 (extends 0038 item 3): a loop that touches memory holds the object a pointer variable designates at entry, with its liveness and token cells, so a block held through a local survives the loop and is freed after it
// Pre-registered from record 0047 item 10 (cut 1: if loops do not land, this file and negative/heap_loop_free_in_body.c wait for d2b): verifies
#include <cerastes.h>
#include <stdlib.h>

int loop_holds_block(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == -1)
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
  free(p);
  return 0;
}
