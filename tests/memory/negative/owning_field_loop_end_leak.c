// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_free_after.c
// Pins: 0055 items 5, 7: the same without the free after the loop: the leaf still holds its block when s ends (d_ok_end_leak.c: its genuine leak)
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): record 0055 item 5: leak at the return statement (d_ok_end_leak.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = i;
    }
    i = i + 1;
  }
  return 0;  // <- here
}
