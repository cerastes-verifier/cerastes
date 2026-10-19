// expect: frontend-error undefined_identifier @ L9
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: the measure names something that is not declared.
// Pins: EDG checks the arguments of decreases like those of requires
#include <cerastes.h>


int f(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(m);
