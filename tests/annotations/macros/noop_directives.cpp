// expect: golden
// expect: native-ok
// Pins: plan R4c: directives that change no macro are not rejected: an #undef of a macro that is not defined, an identical redefinition (EDG records only what the preprocessor changed)
#undef cerastes
#undef requires
#undef CERASTES_H
#undef CERASTES_REQUIRES
#include <cerastes.h>
#define CERASTES_H
#undef ensures
#undef old

int half(int x) CERASTES_REQUIRES(x >= 0);
