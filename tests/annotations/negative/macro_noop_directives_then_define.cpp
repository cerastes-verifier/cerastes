// expect: frontend-error cerastes-annotation_macro_changed @ L14
// expect: native-ok
// twin: tests/annotations/macros/noop_directives.cpp
// Mutation of the twin: the no-op directives of the twin, and one real #define of an attribute name after them.
// Pins: plan R4c: the no-op directives are accepted, a real change next to them is not
#undef cerastes
#undef requires
#undef CERASTES_H
#undef CERASTES_REQUIRES
#include <cerastes.h>
#define CERASTES_H
#undef ensures
#undef old
#define ensures requires

int half(int x) CERASTES_REQUIRES(x >= 0);
