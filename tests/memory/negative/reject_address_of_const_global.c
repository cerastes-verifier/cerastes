// expect: rejected cerastes-unsupported_expression @ L8
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 items 3, 4: the address of a const global (a named constant the encoder folds) is a pointer value that is not a cell, rejected (re-expected by M4.5b2)
#include <cerastes.h>

const int G = 5;
int address_of_global(void) { const int *p = &G; return *p; }
