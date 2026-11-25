// expect: error overflow @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0063 item 14 (flip B of M4.5e1e1, was `rejected unsupported_type` by 0024 item 6): the bound of an array type in a type operand is read as written, so the wrapping 65536u * 65536u + 1 is an overflow at the bound (the front end says char[1]); Pre-registered from record 0063 item 14 (Part D), before any run
#include <cerastes.h>

int huge(void)
{
  return (int)sizeof(char[65536u * 65536u + 1]);
}
