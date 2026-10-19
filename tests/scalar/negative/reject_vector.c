// expect: rejected cerastes-unsupported_type @ L10
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2: a vector type is rejected
// Assumption A4 (INDEX.md): the typedef itself is not diagnosed, its use in the function is.
#include <cerastes.h>

typedef int v4 __attribute__((vector_size(16)));

int first(v4 v) { return 0; }
