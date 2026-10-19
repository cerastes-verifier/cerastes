// expect: frontend-error cerastes-annotation_macro_changed @ L9
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define decreases requires` after the include.
// Pins: plan R4c: annotation_macro_changed (the attribute name decreases is protected like the others)
#include <cerastes.h>


#define decreases requires

int half(int x) CERASTES_REQUIRES(x >= 0);
