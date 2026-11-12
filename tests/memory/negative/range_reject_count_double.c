// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 3 (audit S3): a floating count is rejected by the count rule (a literal 2.0 with an int parameter list: a double parameter would be rejected first, as unsupported_type, before the count is read)
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2.0))
{
  (void)p;
}
