// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a pointer to a pointer is rejected (re-expected by b1c)
#include <cerastes.h>

int pointer_to_pointer(void) { int **pp = 0; return pp == 0; }
