// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5 (lifetime end at a return): return from inside the block of a struct local whose owning leaf was freed first
// Pre-registered from audit round 2 on Parts B/C (probes r3/): ret_in_block.c with the free before the return: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int c)
{
  {
    struct buf s = { NULL, 0 };
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = 1;
    }
    if (c) {
      free(s.data);
      return 1;
    }
    free(s.data);
    s.data = NULL;
  }
  return 0;
}
