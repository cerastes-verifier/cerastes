// expect: rejected cerastes-unsupported_type @ L8
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: the type walk goes through references to the array (UNCONFIRMED: the record lists pointers, typedefs, decltype and alias typerefs; references are my reading)
#include <cerastes.h>

int three_ints(void)
{
  return (int)sizeof(int(&)[3]);
}
