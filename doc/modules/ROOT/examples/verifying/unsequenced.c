// expect: rejected cerastes-unsequenced_modification @ L10
// mode: c23, c++20
// i is modified and read in one expression with nothing between them: the result is undefined, and cerastes refuses it.
// tag::source[]
#include <cerastes.h>

int bad(int i)
{
  // i++ and i are unsequenced
  return i++ + i;
}
// end::source[]
