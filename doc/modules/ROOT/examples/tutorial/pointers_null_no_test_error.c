// expect: error null-dereference @ L13
// mode: c23, c++20
// Without the NULL test p may be null when it is read.
// Derived from pointers_null.c.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int value_or(int *p, int fallback)
    CERASTES_REQUIRES(p == NULL || cerastes_reads(*p))
    CERASTES_ENSURES(r, p == NULL ? r == fallback : (cerastes_reads(*p) && r == *p))
{
  return *p;
}
// end::source[]
