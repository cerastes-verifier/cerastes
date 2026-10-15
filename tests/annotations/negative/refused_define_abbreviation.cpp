// expect: frontend-refused of __CERASTES__ is not supported
// cerastes-fe-options: --c++20 --defi=__CERASTES__=0
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: an abbreviation of the long form (--defi) must be refused too: PENDING, today it is accepted and silently redefines the macro.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
