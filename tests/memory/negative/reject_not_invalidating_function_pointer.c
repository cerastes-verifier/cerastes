// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: not_invalidating on a pointer to a function (M4.7 owns function pointers)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter (as reject_marker_function_pointer.c)
#include <cerastes.h>

void f(CERASTES_NOT_INVALIDATING void (*fp)(void)) { (void)fp; }  // <- here
