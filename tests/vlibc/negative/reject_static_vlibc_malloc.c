// expect: rejected cerastes-annotation_reserved_library @ L9
// expect: frontend-warning declared_but_not_referenced @ L9
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 8 (audit round 2 S3, 0046 item 6): a static vlibc_malloc declared before the header makes vlibc's contract land on the program's own function: rejected, with the vlibc-specific message
// Pre-registered from record 0050 item 8: rejected annotation_reserved_library at the static declaration; the `frontend-warning declared_but_not_referenced` line (EDG #177, a static declaration that nothing refers to) was added after its first run, as owner question 126 did for the earlier tests, and the line numbers moved by one with it

static void *vlibc_malloc(unsigned long size);  // <- here
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  return 0;
}
