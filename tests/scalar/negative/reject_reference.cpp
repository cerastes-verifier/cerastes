// expect: rejected cerastes-unsupported_type @ L6
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2: a reference type is rejected (C++)
#include <cerastes.h>

int get(int &r) { return r; }
