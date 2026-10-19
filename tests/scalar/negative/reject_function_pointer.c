// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a function pointer is rejected
#include <cerastes.h>

int apply(int (*f)(int), int x) { return x; }
