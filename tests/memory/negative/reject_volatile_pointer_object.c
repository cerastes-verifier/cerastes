// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 2: a volatile-qualified pointer (the pointer itself, not its pointee) is rejected (not lifted before M8)
#include <cerastes.h>

int volatile_object(void) { int x = 0; int *volatile p = &x; return *p; }
