// expect: error overflow @ L9
// INT_MIN / -1 is not representable, so it is an overflow, not a division by zero.
// tag::source[]
#include <cerastes.h>

int quotient(int a, int b)
    CERASTES_REQUIRES(b != 0)
{
  return a / b;
}
// end::source[]
