// expect: verified
// expect: native-ok
// Operands narrower than int are promoted: the product is computed in int, then converted back.
// tag::source[]
#include <cerastes.h>

unsigned char scale(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == (a * b) % 256)
{
  return a * b;
}
// end::source[]
