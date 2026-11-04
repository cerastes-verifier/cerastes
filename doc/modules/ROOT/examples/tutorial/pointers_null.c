// expect: verified
// expect: native-ok
// mode: c23, c++20
// A possibly-null pointer is tested before it is read.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int value_or(int *p, int fallback)
    CERASTES_REQUIRES(p == NULL || cerastes_reads(*p))
    CERASTES_ENSURES(r, p == NULL ? r == fallback : (cerastes_reads(*p) && r == *p))
{
  if (p == NULL) {
    return fallback;
  }
  return *p;
}
// end::source[]
