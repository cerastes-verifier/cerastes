// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: the type of a sizeof expression operand is checked (UNCONFIRMED tag: the string literal might be unsupported_expression first)
#include <cerastes.h>

int three_or_four(void)
{
  return (int)sizeof("abc");
}
