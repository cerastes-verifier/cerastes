// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 2: an _Atomic-qualified pointer is rejected (not lifted before M8)
#include <cerastes.h>

int atomic_pointer(void) { int x = 0; int *_Atomic p = &x; return *p; }
