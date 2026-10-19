// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2 and 0012 item 12: a pointer type is rejected
#include <cerastes.h>

int deref(int *p) { return 0; }
