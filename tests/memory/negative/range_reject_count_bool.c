// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 3 (audit S3): after the top-level conversion is peeled, a bool count is rejected (the tag is the encoder's choice: pre-registered as unsupported_annotation like the other count rejections)
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p, bool b)
    CERASTES_REQUIRES(cerastes_owns_range(p, b))
{
  (void)p;
  (void)b;
}
