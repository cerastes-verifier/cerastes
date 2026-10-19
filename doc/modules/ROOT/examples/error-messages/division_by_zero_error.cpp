// expect: error division-by-zero @ L8
// A program that fails with the division-by-zero error.
// tag::source[]
#include <cerastes.h>

unsigned per_item(unsigned total, unsigned count)
{
  return total / count;
}
// end::source[]
