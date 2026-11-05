// expect: error invalid-free @ L36
// expect: error leak @ L39
// expect: error permission @ L39
// expect: error permission @ L39
// expect: error permission @ L39
// expect: error permission @ L39
// expect: error invalid-free @ L41
// expect: error invalid-free @ L41
// expect: error leak @ L43
// mode: c23, c++20
// twin: tests/memory/heap_loop_two_blocks.c
// Pins: 0047 item 10 as amended at M4.5d2v (the false alarm it names): the body frees the block that a variable declared outside the loop holds and allocates another into the same variable, then the frees after the loop name that variable: a block allocated in an iteration and kept in an outer variable is not carried to the next iteration, so the program, which is correct, is reported in user terms and never exits 3, until M4.5d2b's owning variables (the same shape as vlibc/negative/vlibc_alloc_loop_replace_false_alarm.c, with the platform's allocator)
// Pre-registered from the vlibc program's nine errors, then set to the first run's (owner question 145, M4.5d2v: the false alarm pinned as the encoder reports it): without a vlibc view free's token row is not split, so one invalid-free at the free in the body; at the loop body's closing brace a leak and four permission rows (value kept and permission kept, for *p and for *q); two invalid-free at the free after the loop; a leak at the end
#include <stdlib.h>
#include <cerastes.h>

int mixed(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    free(q);
    q = (int *)malloc(sizeof(int));
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
