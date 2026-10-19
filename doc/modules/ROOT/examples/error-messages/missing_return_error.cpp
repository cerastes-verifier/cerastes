// expect: error missing-return @ L14
// A program that fails with the missing-return error.
// tag::source[]
#include <cerastes.h>

int sign(int x)
{
  if (x > 0) {
    return 1;
  }
  if (x < 0) {
    return -1;
  }
}
// end::source[]
