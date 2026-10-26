// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: a wrapping array bound is the unchecked front-end value this record closes (Q32: 'any other known instances')
#include <cerastes.h>

int huge(void)
{
  return (int)sizeof(char[65536u * 65536u + 1]);
}
