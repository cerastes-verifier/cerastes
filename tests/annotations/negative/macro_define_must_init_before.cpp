// expect: frontend-error cerastes-annotation_macro_changed @ L6
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define must_init must_init` before the include.
// Pins: decision 0042 item 2: `must_init` is a protected macro name, before the include as well
#define must_init must_init
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
