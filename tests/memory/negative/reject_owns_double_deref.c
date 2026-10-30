// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 6 (question 74): owns(**pp) names a place reached through a heap read: rejected until M4.5c
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(**pp))
{
  (void)pp;
}
