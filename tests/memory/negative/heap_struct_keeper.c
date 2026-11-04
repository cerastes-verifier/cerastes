// expect: error invalid-free @ L26
// mode: c23, c++20
// twin: tests/memory/heap_struct.c
// Pins: 0047 item 6: a callee kept a liveness fraction of the object through &s->y (its requires has no matching ensures), so the whole-liveness row of the free fails
// Pre-registered from record 0047 item 6 (full liveness: a callee may still hold a pointer): invalid-free at the free, one error
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

void keep_live(int *q)
    CERASTES_REQUIRES(cerastes_live(q));

int use_struct(void)
{
  struct S *s = (struct S *)malloc(sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  s->x = 1;
  s->y = 2;
  keep_live(&s->y);
  free(s);  // <- here
  return 0;
}
