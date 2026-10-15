// expect: golden
// expect: native-ok
// A check after an if needs braces, or the ordinary compiler sees an empty statement.
#include <cerastes.h>

// tag::braces[]
int f(int x)
{
  if (x > 0) {
    CERASTES_CHECK(x > 0);
  }
  return x;
}
// end::braces[]
