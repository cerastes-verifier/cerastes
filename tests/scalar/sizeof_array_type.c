// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1, the program unchanged, line for line, was reject_sizeof_array.c, rejected by 0024 item 6 until now): sizeof of an array type: the bound is read as written and the value is the front end's, 3 * 4 = 12 (twin negative/sizeof_array_type_wrong.c)
#include <cerastes.h>

int three_ints(void)
{
  return (int)sizeof(int[3]);
}
