// expect: rejected cerastes-unsupported_declaration @ L6
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a function template in a verified file is rejected (M4)
#include <cerastes.h>

template <typename T> T identity(T x) { return x; }
