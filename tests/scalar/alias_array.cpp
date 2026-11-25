// expect: verified
// expect: native-ok
// Pins: 0063 item 14 (flip A of M4.5e1e1, was reject_alias_array.cpp): a using-alias of an array type in a verified file, its bound checked as written; unchanged, line for line (twin negative/alias_array_wrap.cpp)
#include <cerastes.h>

using buf_t = char[4];

int zero(void)
{
  return 0;
}
