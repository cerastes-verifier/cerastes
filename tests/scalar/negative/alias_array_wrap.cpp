// expect: error overflow @ L7
// twin: tests/scalar/alias_array.cpp
// Pins: 0063 item 14: the bound of a using-alias of an array type is checked as written at the alias: overflow at the bound
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

using buf_t = char[65536u * 65536u + 1];  // <- here

int zero(void)
{
  return 0;
}
