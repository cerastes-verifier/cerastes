// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_goto_out.c
// Pins: 0055 item 5: the goto leaves the block while s.data still holds its block: "owning field `s.data` still holds its block when `s`'s lifetime ends", at the goto (goto_out.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): goto_out.c: leak at the goto
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
      goto out;  // <- here
    }
    free(s.data);
    s.data = NULL;
  }
out:
  return 0;
}
