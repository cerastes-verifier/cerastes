// expect: rejected cerastes-unsupported_declaration @ L6
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: using-aliases are typedef entries in the IL
#include <cerastes.h>

using buf_t = char[4];

int zero(void)
{
  return 0;
}
