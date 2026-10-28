// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a pointer parameter is rejected (re-expected by b1b)
#include <cerastes.h>

int deref(int *p) { return 0; }
