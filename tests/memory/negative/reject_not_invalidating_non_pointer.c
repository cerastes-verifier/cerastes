// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: not_invalidating on a non-pointer parameter (never lifted)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>

void f(int n CERASTES_NOT_INVALIDATING) { (void)n; }  // <- here
