// expect: frontend-error cerastes-annotation_macro_changed @ L6
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#undef __LINE__` after the include: a predefined macro of the front end.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#undef __LINE__

int half(int x) CERASTES_REQUIRES(x >= 0);
