// expect: error leak @ L25
// mode: c23, c++20
// twin: tests/memory/heap_loop_member_param.c
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): the function takes the block and does not free it: leak at the closing brace
// Pre-registered from .claude/plans/m4-5d2v-allocator.md Part 1 (t16's twin: no free(s)): leak at the function's closing brace
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int a;
  int b;
};

void g(struct S *s, int n)
    CERASTES_REQUIRES(cerastes_owns_block(s) && 0 <= n && n <= 100)
{
  int *x = &s->b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *x = i;
    i += 1;
  }
}  // <- here
