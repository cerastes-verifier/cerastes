// expect: rejected cerastes-unsupported_type @ L8
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: __datasizeof operands follow the type rule. Probe the front end first: if EDG refuses the keyword, drop this draft
#include <cerastes.h>

int data(void)
{
  return (int)__datasizeof(int[3]);
}
