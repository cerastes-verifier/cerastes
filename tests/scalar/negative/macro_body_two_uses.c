// expect: error overflow @ L15
// expect: error overflow @ L21
// mode: c23, c++20
// twin: tests/scalar/macro_body.c
// Pins: 0012 item 13: the macro body overflows at two uses, reported at the two use lines
// Each error is at the line of its use. The driver also prints, after each one, a note at the macro's definition:
// `<file>:<line of the #define>: note: in the macro (this is where the macro defines it)` (the harness ignores notes).
#include <cerastes.h>

#define SCALE(v) ((v) * 1000)

int first(int a)
    CERASTES_REQUIRES(a >= 0 && a <= 10000000)
{
  return SCALE(a);
}

int second(int b)
    CERASTES_REQUIRES(b >= 0 && b <= 10000000)
{
  return SCALE(b);
}
