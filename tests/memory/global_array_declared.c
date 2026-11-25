// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 1 and 9 (flip A of M4.5e1e1, was global_array.c, rejected by 0041 item 2 until now): an array global with external linkage and no initializer is a declared one-block global; the program is unchanged, line for line (twin negative/global_array_unknown_check.c)
#include <cerastes.h>

int values[3];

int ident(int a)
{
  return a;
}
