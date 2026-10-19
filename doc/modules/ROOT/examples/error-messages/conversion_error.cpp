// expect: error conversion @ L9
// A program that fails with the conversion error.
// tag::source[]
#include <cerastes.h>

signed char to_small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 200)
{
  return (signed char)x;
}
// end::source[]
