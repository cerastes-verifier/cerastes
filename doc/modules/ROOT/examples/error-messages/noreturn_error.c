// expect: error noreturn @ L14
// mode: c23, c++20
// A program that fails with the noreturn error.
// tag::source[]
#include <stdlib.h>
#include <cerastes.h>

// Ends the program when bad is not zero.
[[noreturn]] void fail_if(int bad)
{
  if (bad) {
    abort();
  }
}
// end::source[]
