// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_local_exits.c
// Pins: 0052 item 3: break leaves the loop body with the owning local's block held: leak at the break
// Pre-registered from record 0052 item 3: leak at the break
#include <cerastes.h>
#include <stdlib.h>

int scan(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      return -1;
    }
    *p = i;
    if (i == 5) {
      break;  // <- here
    }
    free(p);
    i += 1;
  }
  return 0;
}
