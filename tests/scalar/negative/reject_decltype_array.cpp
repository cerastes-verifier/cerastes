// expect: rejected cerastes-unsupported_type @ L9
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: decltype operands follow the type rule; "abc" is an lvalue of type const char[4], so decltype gives a reference to an array (the helper walks references).
// UNCONFIRMED: the string literal itself might be rejected first (unsupported_expression); see INDEX open questions
#include <cerastes.h>

int declared(void)
{
  return (int)sizeof(decltype("abc"));
}
