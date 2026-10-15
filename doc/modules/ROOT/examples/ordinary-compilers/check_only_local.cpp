// expect: golden
// expect: native-ok
// A local that only a check uses: mark it for the ordinary compiler.
#include <cerastes.h>

// tag::local[]
int twice(int x)
{
  [[maybe_unused]] int before = x;
  int r = x + x;
  CERASTES_CHECK(r == before + before);
  return r;
}
// end::local[]
