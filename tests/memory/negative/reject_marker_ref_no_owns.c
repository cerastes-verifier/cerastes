// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 4 (question 88): ref_to_uninit without an owns(*p) atom in the requires (never lifted)
#include <cerastes.h>

void f(int *p CERASTES_REF_TO_UNINIT)  // <- here
{
  (void)p;
}
