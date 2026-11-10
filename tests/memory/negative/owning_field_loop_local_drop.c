// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_local.c
// Pins: 0055 items 5, 7: the loop re-allocates s.data without freeing the block it holds from the second iteration on: the reassignment row inside the loop, "assigning to owning field `s.data` drops the block it holds", at the store (owning_global_loop_cycle_no_free.c)
// Pre-registered from record 0055 item 7 (loops over a named leaf: new, no base form; S40 (g) g2_named*.c are today's rejection and false alarm): leak at the store
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int churn(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s.data = (int *)malloc(sizeof(int));  // <- here
    if (s.data == NULL) {
      return 1;
    }
    *s.data = i;
    i += 1;
  }
  free(s.data);
  return 0;
}
