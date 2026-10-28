// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a pointer result is rejected (re-expected by b1b)
#include <cerastes.h>

int *none(void) { return 0; }
