// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// An enumeration value in arithmetic is rejected: C converts it to unsigned int and pass:[C++] to int.
// tag::source[]
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int plus_one(enum color c)
{
  return c + 1;
}
// end::source[]
