// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 2: void* is rejected (not lifted by b1b, b1c or M4.5d2a: void* stays restricted to an allocation's converted result and free's argument, record 0047 item 11; M4.7 and M8)
#include <cerastes.h>

int void_pointer(void) { void *v = 0; return v == 0; }
