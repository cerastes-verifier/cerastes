// expect: error invalid-free @ L30
// mode: c23, c++20
// twin: tests/memory/heap_loop_member_alias.c
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): the block the member alias points into is freed twice: the second free finds no token
// Pre-registered from .claude/plans/m4-5d2v-allocator.md Part 1 (t22's twin: a double free): invalid-free at the second free only
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int a;
  int b;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct S *s = (struct S *)calloc(1, sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  int *x = &s->b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *x = i;
    i += 1;
  }
  free(s);
  free(s);  // <- here
  return 0;
}
