// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 2 of M4.5b1a, 0032 item 2: a global of pointer type is rejected (lifted by M4.5b2, globals)
#include <cerastes.h>

int *gp;

int ident(int a) { return a; }
