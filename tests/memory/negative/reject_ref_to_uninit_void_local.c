// expect: rejected cerastes-unsupported_annotation @ L12
// expect: rejected cerastes-unsupported_type @ L13
// mode: c23, c++20
// twin: tests/memory/ref_to_uninit_local.c
// Pins: 0052 item 4 (permanent): cerastes::ref_to_uninit on a void* local (R3 allows void* markers; cerastes keeps the markers on typed pointers, 0042 item 4)
// Pre-registered from record 0052 item 4: rejected unsupported_annotation at the declaration (tag guessed as for reject_owning_void_local.c)
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  CERASTES_REF_TO_UNINIT void *p = NULL;  // <- here
  return p == NULL ? 0 : 1;  // <- here
}
