// expect: golden
// expect: native-ok
// Pins: requires on a declaration with no body: one attribute on the routine,
// parameters resolved as param_ref, the condition typed bool.
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
