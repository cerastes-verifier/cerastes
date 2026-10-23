// expect: verified
// expect: native-ok
// Operands narrower than int are promoted: the product is computed in int, and reduced explicitly before it converts back.
// tag::source[]
#include <cerastes.h>

unsigned char scale(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == (a * b) % 256)
{
  return (unsigned char)((a * b) % 256);
}
// end::source[]
