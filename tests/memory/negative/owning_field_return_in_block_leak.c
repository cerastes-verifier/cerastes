// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_return_in_block.c
// Pins: 0055 item 5: the return leaves the block while s.data still holds its block, at the return (ret_in_block.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): ret_in_block.c: leak at the return
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
      return 1;  // <- here
    }
    free(s.data);
    s.data = NULL;
  }
  return 0;
}
