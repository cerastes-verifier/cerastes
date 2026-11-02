// expect: frontend-error cerastes-annotation_macro_changed @ L7
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define owning owning` after the include.
// Pins: decision 0042 item 2: `owning` is a protected macro name (a user `#define` would rewrite the attribute token of the marker)
#include <cerastes.h>
#define owning owning

int half(int x) CERASTES_REQUIRES(x >= 0);
