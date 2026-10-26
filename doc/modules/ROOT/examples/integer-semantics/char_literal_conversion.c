// expect: error conversion @ L9
// mode: c23, c++20
// '\xff' is -1 in a plain char (and in C an int -1): the value wraps, so cerastes reports a conversion at the literal. The fix is the number 255 (or u8'\xff' in C23).
// tag::source[]
#include <cerastes.h>

unsigned char all_ones(void)
{
  unsigned char c = '\xff';
  return c;
}
// end::source[]
