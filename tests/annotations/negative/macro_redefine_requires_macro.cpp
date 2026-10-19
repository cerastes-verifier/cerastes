// expect: frontend-error cerastes-annotation_macro_changed @ L8
// expect: frontend-warning bad_macro_redef @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: CERASTES_REQUIRES is redefined (differently: to nothing) after the include.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#define CERASTES_REQUIRES(...)

int half(int x) CERASTES_REQUIRES(x >= 0);
