// expect: error overflow @ L8
// mode: c23, c++20
// twin: tests/scalar/typedef_array_file.c
// Pins: 0063 item 14: the bound of a typedef of an array type is checked as written at the typedef: 65536u * 65536u wraps (the front end says char[1]); overflow at the bound
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

typedef char buf_t[65536u * 65536u + 1];  // <- here

int zero(void)
{
  return 0;
}
