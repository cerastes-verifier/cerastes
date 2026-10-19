// expect: frontend-error cerastes-annotation_macro_changed @ L7
// expect: frontend-warning bad_macro_redef @ L7
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define CERASTES_H 1` after the include (a different definition from the header's).
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#define CERASTES_H 1

int half(int x) CERASTES_REQUIRES(x >= 0);
