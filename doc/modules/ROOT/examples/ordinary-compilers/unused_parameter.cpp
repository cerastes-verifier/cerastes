// expect: golden
// expect: native-ok
// A parameter that only a contract uses: mark it for the ordinary compiler.
#include <cerastes.h>

// tag::param[]
int first(const int* a, [[maybe_unused]] int n) CERASTES_REQUIRES(n > 0)
{
  return a[0];
}
// end::param[]
