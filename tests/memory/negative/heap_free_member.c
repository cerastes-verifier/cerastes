// expect: error invalid-free @ L22
// mode: c23, c++20
// twin: tests/memory/heap_free_kinds.c
// Pins: 0047 item 6: a member of an allocation is not the start of one: its block has no token (the member's liveness is the enclosing object's and whole)
// Pre-registered from record 0047 item 6: invalid-free at the call, one error; the failed token row ends the path, so s is not reported as a leak
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

int free_member(void)
{
  struct S *s = (struct S *)malloc(sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  s->x = 1;
  s->y = 2;
  free(&s->y);  // <- here
  return 0;
}
