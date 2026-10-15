// expect: frontend-error attr_result_name_void_function @ L12
// expect: native-ok
// twin: tests/annotations/contracts/void_variants.cpp
// Mutation of the twin: a result name on a function returning void through an alias (CP2a: typedef'd void).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

using unit = void;

void clear(int x) CERASTES_ENSURES(x > 0);

unit reset(int x) CERASTES_ENSURES(r, x > 0);
