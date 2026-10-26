// expect: rejected cerastes-unsupported_expression @ L6
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 2 / Consequences: noexcept is not encoded and is never evaluated by the skip scan, so it goes to the encoder's rejections
#include <cerastes.h>

static_assert(noexcept(1 + 1));

int one()
{
  return 1;
}
