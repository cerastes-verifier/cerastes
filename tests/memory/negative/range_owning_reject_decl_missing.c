// expect: rejected cerastes-annotation_mismatch @ L11
// mode: c23, c++20
// twin: tests/memory/range_owning_param.c
// Pins: 0060 item 13: one declaration gives owning(n) and a later one gives none: annotation_mismatch at the later declaration
// Pre-registered from record 0060 item 13 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int n, int *p CERASTES_OWNING_N(n));

void f(int n, int *p)
{
  (void)n;
  (void)p;
}
