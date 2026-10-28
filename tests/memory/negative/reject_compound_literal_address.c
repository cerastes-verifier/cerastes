// expect: rejected cerastes-unsupported_expression @ L7
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 4: the address of a compound literal is not the address of a cell, rejected (not lifted before M4.5c)
#include <cerastes.h>

int compound_literal(void) { int *p = &(int){5}; return *p; }
