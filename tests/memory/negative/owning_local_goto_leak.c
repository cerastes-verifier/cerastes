// expect: error leak @ L19
// mode: c23, c++20
// twin: tests/memory/owning_local_exits.c
// Pins: 0052 item 3: a goto out of the owning local's block with its block held: leak at the goto
// Pre-registered from record 0052 item 3: leak at the goto
#include <cerastes.h>
#include <stdlib.h>

int early(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  {
    CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      return -1;
    }
    *p = c;
    if (c == 0) {
      goto done;  // <- here
    }
    free(p);
  }
done:
  return 0;
}
