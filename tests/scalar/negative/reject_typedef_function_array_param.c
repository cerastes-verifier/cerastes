// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: function types are walked through each parameter's declared type; the declared type of `int a[3]` keeps the array (it is not the adjusted pointer), so the typedef is rejected (observed with the encoder, M4.4b2)
#include <cerastes.h>

typedef int fn_t(int a[3]);

int zero(void)
{
  return 0;
}
