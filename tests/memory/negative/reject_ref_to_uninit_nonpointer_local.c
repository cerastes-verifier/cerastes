// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/ref_to_uninit_local.c
// Pins: 0052 item 4 (permanent): cerastes::ref_to_uninit on a local that is not a pointer
// Pre-registered from record 0052 item 4: rejected unsupported_annotation at the declaration
#include <cerastes.h>

int f(void)
{
  CERASTES_REF_TO_UNINIT int x = 0;  // <- here
  return x;
}
