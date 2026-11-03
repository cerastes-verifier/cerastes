// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 6 (question 74), record 0043 item 15 (re-messaged by M4.5c1): owns(**pp) names a place reached through a heap read: rejected until M4.5c2b (record 0044 item 1: places reached through a heap read)
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(**pp))
{
  (void)pp;
}
