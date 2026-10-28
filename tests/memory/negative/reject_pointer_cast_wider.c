// expect: rejected cerastes-unsupported_expression @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 3: a cast between pointers to long and long long is rejected although the types have one width (not lifted before M8)
#include <cerastes.h>

int wider_pointer(void) { long l = 1; long long *q = (long long *)&l; return q == 0; }
