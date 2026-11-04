// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/heap_loop_param_block.c
// Pins: 0047 item 10: the function takes the block (owns_block in both lists) and frees it after its loop, so the ensures' block is not handed back
// Pre-registered from the loops branch's positive (record 0047 item 10), written after it: the expectation was verified once on the single file before it was written
#include <cerastes.h>
#include <stdlib.h>

struct acc {
  int bal;
  int n;
};

void bump(struct acc *a, int n)
    CERASTES_REQUIRES(cerastes_owns_block(a))
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns_block(a))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a->bal = i;
    a->n = a->bal;
    i += 1;
  }
  free(a);
}  // <- here

int use(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct acc *a = (struct acc *)calloc(1, sizeof(struct acc));
  if (a == NULL) return -1;
  bump(a, n);
  free(a);
  return 0;
}
