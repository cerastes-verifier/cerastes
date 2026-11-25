// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1, the program unchanged, line for line, was reject_alignof_array.c): alignof of an array type, its bound read as written, the value the front end's (1 for char[4]); twin negative/alignof_array_wrong.c
#include <cerastes.h>

int align(void)
{
  return (int)alignof(char[4]);
}
