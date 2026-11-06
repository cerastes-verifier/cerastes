// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/strict_marked_local_alloc.c
// Pins: 0052 item 4: under --strict-init an unmarked local assigned malloc's result is rejected like one initialized from it (reject_strict_init_malloc_local.c), with P4222R3's wording
// Pre-registered from record 0052 item 4: rejected unsupported_annotation at the call (as reject_strict_init_malloc_local.c)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *p = NULL;
  p = (int *)malloc(sizeof(int));  // <- here
  free(p);
  return 0;
}
