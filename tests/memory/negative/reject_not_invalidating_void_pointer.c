// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: not_invalidating on a void * (never lifted)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>

void f(void *p CERASTES_NOT_INVALIDATING) { (void)p; }  // <- here
