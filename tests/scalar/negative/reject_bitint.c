// expect: rejected cerastes-unsupported_type @ L7
// c-only: _BitInt is C23
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2: _BitInt(N) is rejected (C23)
#include <cerastes.h>

_BitInt(8) pass(_BitInt(8) x) { return x; }
