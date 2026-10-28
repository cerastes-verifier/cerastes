// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 2: a volatile pointee is rejected (not lifted by b1b or b1c: re-expected by M8)
#include <cerastes.h>

int volatile_pointer(void) { volatile int v = 0; volatile int *p = &v; return p == 0; }
