// expect: verified
// expect: native-ok
// mode: c23, c++20
// A character literal is the number it names: 'a' is 97, '\x7f' is 127. A byte above 127 is written as a number.
// tag::source[]
#include <cerastes.h>

int letter_a(void)
    CERASTES_ENSURES(r, r == 97)
{
  return 'a';
}

int last_ascii(void)
    CERASTES_ENSURES(r, r == 127)
{
  return '\x7f';
}

unsigned char all_ones(void)
    CERASTES_ENSURES(r, r == 255)
{
  return 255;
}
// end::source[]
