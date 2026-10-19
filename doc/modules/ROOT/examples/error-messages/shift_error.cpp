// expect: error shift @ L8
// A program that fails with the shift error.
// tag::source[]
#include <cerastes.h>

unsigned bit(unsigned n)
{
  return 1u << n;
}
// end::source[]
