// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2: floating point is rejected
#include <cerastes.h>

float half(float x) { return x; }
