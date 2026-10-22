// expect: error conversion @ L11
// mode: c23, c++20
// Any int may reach the conversion, so the value may not be one of the values of the enumeration.
// tag::source[]
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color from_int(int x)
{
  return (enum color)x;
}
// end::source[]
