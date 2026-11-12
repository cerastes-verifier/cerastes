// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 1 (no row lifts it): a range over a function pointer is rejected where the parameter's type is (unsupported_type at the declaration)
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(void (*p)(void), int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}
