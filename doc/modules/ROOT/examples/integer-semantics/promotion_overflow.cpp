// expect: error overflow @ L8
// unsigned short operands are promoted to int, so their product can overflow int.
// tag::source[]
#include <cerastes.h>

int product(unsigned short a, unsigned short b)
{
  return a * b;
}
// end::source[]
