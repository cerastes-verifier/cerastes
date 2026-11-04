// expect: error invalid-free @ L18
// mode: c23, c++20
// twin: tests/memory/heap_free_kinds.c
// Pins: 0047 item 6: a function that only borrows the object (owns(*s) in both clauses) holds no token, so it cannot free it
// Pre-registered from record 0047 item 6: invalid-free at the call, one line (the token row fails first; whether the path then continues with it assumed and the liveness row, which also fails here, adds a second invalid-free line with another message is for the first run to settle); re-registered after audit round 1 (record 0047 item 6): left at one line as instructed
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

void drop(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*s))
    CERASTES_ENSURES(cerastes_owns(*s))
{
  free(s);  // <- here
}
