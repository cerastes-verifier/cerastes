// expect: frontend-error cerastes-cerastes_macro_changed @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#undef __CERASTES__` inside a skipped `#if 0` branch. The rule is textual and covers every line of the translation unit, taken or not.
// Pins: only this diagnostic is reported (the branch is never taken, so the macros still expand).
#include <cerastes.h>
#if 0
#undef __CERASTES__
#endif

int half(int x) CERASTES_REQUIRES(x >= 0);
