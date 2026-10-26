// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: the type walk goes through pointers (sizeof(char(*)[4]) is 8, the array inside is still an unchecked layout)
#include <cerastes.h>

int ptr(void)
{
  return (int)sizeof(char(*)[4]);
}
