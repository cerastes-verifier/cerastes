// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: typedefs of array type in verified files are unsupported_declaration (until M4.5e)
#include <cerastes.h>

typedef char buf_t[4];

int zero(void)
{
  return 0;
}
