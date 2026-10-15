// expect: frontend-error wrong_entity_for_attribute @ L11
// expect: native-ok
// twin: tests/annotations/contracts/returns_function_pointer.cpp
// Mutation of the twin: after the OUTER parameter list d would bind to the returned function type's parameter (CP2a 2); the strict target check must reject it (verify the tag: any placement diagnostic).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

using handler = int (*)(double);
using handler_ref = int (&)(double);

int (*pick(int x))(double d) CERASTES_REQUIRES(d > 0);

handler make(int x) CERASTES_ENSURES(r, r != nullptr);

handler_ref refer(int x) CERASTES_REQUIRES(x > 0);
