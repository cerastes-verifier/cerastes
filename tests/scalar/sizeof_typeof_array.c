// expect: verified
// expect: native-ok
// mode: c23
// c-only: typeof is a C23 keyword; the C++ spelling is decltype (reject_decltype_array.cpp)
// Pins: 0063 item 14 (flip A of M4.5e1e1, the program unchanged, line for line, was reject_sizeof_typeof_array.c): the operand of sizeof is typeof(int[3]): 12; twin negative/sizeof_typeof_array_wrong.c
#include <cerastes.h>

int typed(void)
{
  return (int)sizeof(typeof(int[3]));
}
