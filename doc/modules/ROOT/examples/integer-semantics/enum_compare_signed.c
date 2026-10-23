// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int same(enum color c, int x)
{
  return c == x;
}

int same_fixed(enum color c, int x)
{
  return (int)c == x;
}
// end::source[]
