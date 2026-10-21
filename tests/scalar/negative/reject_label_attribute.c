// expect: rejected cerastes-unrecognized_attribute @ L15
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 3 and 0011 item 7: an attribute on a label is held to the allow-list; an unknown one is rejected
// The attribute is kept on the label entity, not on the statement; the encoder reads it there.
#include <cerastes.h>

int labeled(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    goto M;
  }
  x += 1;
  [[foo::bar]] M:
  return x;
}
