// expect: rejected cerastes-unsupported_expression @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 3: a string literal as a pointer value is rejected (re-expected by M4.6)
#include <cerastes.h>

int string_pointer(void) { const char *s = "abc"; return s == 0; }
