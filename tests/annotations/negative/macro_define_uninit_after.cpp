// expect: frontend-error cerastes-annotation_macro_changed @ L7
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define uninit uninit` after the include.
// Pins: decision 0042 item 2: `uninit` is a protected macro name (a user `#define` would rewrite the attribute token of the marker)
#include <cerastes.h>
#define uninit uninit

int half(int x) CERASTES_REQUIRES(x >= 0);
