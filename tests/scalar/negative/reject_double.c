// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2: double is rejected
#include <cerastes.h>

double twice(double x) { return x; }
