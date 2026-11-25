// expect: rejected cerastes-call_in_annotation @ L16
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: a call of anything that is not a pure function in the body of a read pure function is call_in_annotation
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the construct's own line: the function is read by its requires alone)
#include <cerastes.h>

int plain(int x)
{
  return x;
}

CERASTES_PURE bool g(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return plain(x) > 0;
}

int use(int y)
{
  return y;
}
