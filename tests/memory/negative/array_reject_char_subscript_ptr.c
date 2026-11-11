// expect: rejected cerastes-char_subscripts @ L12
// mode: c23, c++20
// twin: tests/memory/array_ptr_param_first.c
// Pins: 0059 item 4 (audit round 1, item 6): the plain char index is rejected on a pointer too, as both compilers warn on p[c] at -Wall (the matrix); from Part D
// Pre-registered from record 0059 item 4 as updated by audit round 1 (Part D)
#include <cerastes.h>

int pick(int *p, char c)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= c && c < 1)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p[c];
}
