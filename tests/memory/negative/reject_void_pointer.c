// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 2: void* is rejected (not lifted by b1b or b1c: re-expected by M4.5d2)
#include <cerastes.h>

int void_pointer(void) { void *v = 0; return v == 0; }
