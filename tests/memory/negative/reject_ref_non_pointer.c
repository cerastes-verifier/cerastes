// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 4: ref_to_uninit on a non-pointer parameter (never lifted)
#include <cerastes.h>

void f(int n CERASTES_REF_TO_UNINIT) { (void)n; }  // <- here
