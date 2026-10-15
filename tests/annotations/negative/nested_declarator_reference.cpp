// expect: frontend-error wrong_entity_for_attribute @ L15
// expect: native-ok
// twin: tests/annotations/contracts/returns_function_pointer.cpp
// Mutation of the twin: the same with a reference to function (CP2a 2).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

using handler = int (*)(double);
using handler_ref = int (&)(double);

handler pick(int x) CERASTES_REQUIRES(x > 0);

handler make(int x) CERASTES_ENSURES(r, r != nullptr);

int (&refer(int x))(double d) CERASTES_REQUIRES(d > 0);
