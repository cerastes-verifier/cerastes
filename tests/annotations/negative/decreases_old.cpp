// expect: frontend-error cerastes-old_outside_ensures @ L9
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: the measure names cerastes::old.
// Pins: 0006 item 10: old only in ensures
#include <cerastes.h>


int f(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(cerastes::old(n));
