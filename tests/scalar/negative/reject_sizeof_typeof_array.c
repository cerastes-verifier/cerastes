// expect: rejected cerastes-unsupported_type @ L10
// mode: c23
// twin: tests/scalar/sizeof_value.c
// c-only: typeof is a C23 keyword; the C++ spelling is decltype (reject_decltype_array.cpp)
// Pins: 0024 item 6: typeof operands follow the type rule
#include <cerastes.h>

int typed(void)
{
  return (int)sizeof(typeof(int[3]));
}
