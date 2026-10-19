// expect: rejected cerastes-call_in_annotation @ L14
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0011 item 7: a call inside an annotation is rejected unless it is cerastes::old
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  return x;
}

int use(int y)
    CERASTES_REQUIRES(pos(y) > 0)
{
  return y;
}
