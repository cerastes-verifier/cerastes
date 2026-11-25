// expect: rejected cerastes-unsupported_annotation @ L9
// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: 0063 item 2 (0059 items 1 and 2 for a global array): owning on an array of pointers (M4.8: an array of owning pointers needs a count) and ref_to_uninit on an array (a marker is on pointers) are rejected at the declaration
// Pre-registered from record 0063 item 2 (Part B), before any run: rejected
#include <cerastes.h>

CERASTES_OWNING int *own[2];  // <- here
CERASTES_REF_TO_UNINIT int ru[2];  // <- here

int ident(int a)
{
  return a;
}
