// expect: frontend-error cerastes-cerastes_macro_changed @ L6
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define __CERASTES__ 1` after the include. The value equals the front end's own, so EDG's incompatible-redefinition warning stays silent and only the back end's text rule fires.
// Pins: only this diagnostic is reported. No native-ok: a native-ok test cannot define __CERASTES__ (that would switch its own ordinary-compiler check off).
#include <cerastes.h>
#define __CERASTES__ 1

int half(int x) CERASTES_REQUIRES(x >= 0);
