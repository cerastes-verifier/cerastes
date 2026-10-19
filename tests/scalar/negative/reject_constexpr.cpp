// expect: rejected cerastes-unsupported_declaration @ L6
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a constexpr function in verified code is rejected (a compile-time call would skip its checks)
#include <cerastes.h>

constexpr int square(int x) { return x * x; }
