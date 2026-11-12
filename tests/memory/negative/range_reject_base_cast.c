// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 3 (audit S3): a written cast on the base is rejected: the element type is the parameter's pointee
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range((char *)p, n))
{
  (void)p;
  (void)n;
}
