// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owning_field_break_out.c
// Pins: 0055 item 5: the break leaves the body while s.data still holds its block, at the break (brk.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): brk.c: leak at the break
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    struct buf s = { NULL, 0 };
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = 1;
    }
    if (i == 1) {
      break;  // <- here
    }
    free(s.data);
    s.data = NULL;
    i = i + 1;
  }
  return i;
}
