// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/heap_strict_init_free.c
// Pins: 0047 item 8: under --strict-init a call of malloc stored into a member (`s->q = malloc(...)`) is rejected, naming d2b
// Pre-registered from record 0047 item 8: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue); re-registered after audit round 1 (record 0047 item 8): audit S6: wherever it stands
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int *q;
};

void f(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(s->q))
    CERASTES_ENSURES(cerastes_owns(s->q))
{
  s->q = (int *)malloc(sizeof(int));  // <- here
}
