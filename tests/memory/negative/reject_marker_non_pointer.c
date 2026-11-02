// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4: must_init on a non-pointer parameter (never lifted)
#include <cerastes.h>

void f(int n CERASTES_MUST_INIT) { (void)n; }  // <- here
