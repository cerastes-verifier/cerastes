// expect: frontend-error attr_result_name_not_allowed @ L13
// expect: native-ok
// twin: tests/annotations/contracts/returns_function_pointer.cpp
// Mutation of the twin: a result name after a nested declarator; EDG may also report the placement (wrong_entity_for_attribute) at the same line (pin on the first run).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

using handler = int (*)(double);
using handler_ref = int (&)(double);

handler pick(int x) CERASTES_REQUIRES(x > 0);

int (*make(int x))(double d) CERASTES_ENSURES(r, r != nullptr);

handler_ref refer(int x) CERASTES_REQUIRES(x > 0);
