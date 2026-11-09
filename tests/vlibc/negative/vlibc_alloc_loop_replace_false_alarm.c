// expect: error invalid-free @ L37
// expect: error invalid-free @ L37
// expect: error invalid-free @ L37
// expect: error leak @ L40
// expect: error permission @ L40
// expect: error permission @ L40
// expect: error invalid-free @ L42
// expect: error invalid-free @ L42
// expect: error leak @ L44
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_mixed_loop.c
// Pins: 0047 item 10 as amended at M4.5d2v (the false alarm it names, with a vlibc block): the body frees the block that a variable declared outside the loop holds and allocates another into the same variable, then the frees after the loop name that variable: a block allocated in an iteration and kept in an outer variable is not carried to the next iteration (the places are the blocks the variables held at entry), so the program, which is correct, is reported in user terms (invalid-free at the free in the body and after the loop, leak at the back edge and at the end, permission at the back edge) and never exits 3, until M4.5d2b's owning variables; M4.5d2b2a (record 0054 item 4): the same loop with a written owns_block invariant verifies (a written cerastes_owns_vlibc_block invariant or an owning local); this pin, with no invariant, stays
// Pre-registered from il-analyst's fixed encoder on this program (M4.5d2v, the owner's decision to pin it): nine errors, three invalid-free at the free in the body, a leak and two permission at the loop body's closing brace, two invalid-free at the free after the loop, a leak at the end of the function
#include <vlibc/stdlib.h>
#include <cerastes.h>

int mixed(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)vlibc_malloc(sizeof(int));
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
    vlibc_free(q);
    q = (int *)vlibc_malloc(sizeof(int));
    i += 1;
  }
  free(p);
  vlibc_free(q);
  return 0;
}
