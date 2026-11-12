// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/range_marker_must_init.c
// Pins: 0060 item 12 (as 0042 item 4 for reads(*p)): reads_range beside a marker is rejected: the marker needs owns_range(p, n) as its atom
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_reads_range(p, 3))
{
  (void)p;
}
