// expect: error overflow @ L8
// A program that fails with the overflow error.
// tag::source[]
#include <cerastes.h>

int increment(int x)
{
  return x + 1;
}
// end::source[]
