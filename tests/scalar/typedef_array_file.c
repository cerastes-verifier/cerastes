// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1, was reject_typedef_array_file.c): a typedef of an array type in a verified file, its bound checked as written at the typedef; unchanged, line for line (twin negative/typedef_array_file_wrap.c)
#include <cerastes.h>

typedef char buf_t[4];

int zero(void)
{
  return 0;
}
