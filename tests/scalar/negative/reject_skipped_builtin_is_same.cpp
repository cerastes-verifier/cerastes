// expect: rejected cerastes-unsupported_type @ L7
// twin: tests/scalar/static_assert_skipped.cpp
// Pins: 0024 item 3: every type operand of a builtin operation is held to the type rule even though the assertion is skipped
#include <cerastes.h>


static_assert(__is_same(char[4], char[4]), "array operands");

int one()
{
  return 1;
}
