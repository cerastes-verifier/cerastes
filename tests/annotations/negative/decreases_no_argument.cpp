// expect: frontend-error exp_primary_expr @ L9
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: decreases with no measure: a measure has one or more expressions (the empty measure is the encoder's, not the user's).
// Pins: decision 0013 item 1: signature (X+)
#include <cerastes.h>


int f(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES();
