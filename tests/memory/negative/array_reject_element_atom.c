// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1, 0060 item 1 (flip B): an atom on an element (owns(p[1])) is rejected; the text now names M4.5e2 (owner's question 236: atoms on one element come with pointer arithmetic), not e1b
// Pre-registered from record 0059 item 1
#include <cerastes.h>

int f(int *p)
    CERASTES_REQUIRES(cerastes_owns(p[1]))
{
  return 0;
}
