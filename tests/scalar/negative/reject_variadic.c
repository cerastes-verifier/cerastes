// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a variadic function is rejected
#include <cerastes.h>

int sum_all(int n, ...) { return n; }
