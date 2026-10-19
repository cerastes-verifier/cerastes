// expect: frontend-error attr_redeclaration_mismatch @ L10
// expect: native-ok
// twin: tests/annotations/contracts/decreases_redecl.cpp
// Mutation of the twin: a redeclaration with another measure.
// Pins: 0006 item 6 for decreases (must match across declarations)
#include <cerastes.h>


int f(int n) CERASTES_DECREASES(n);
int f(int n) CERASTES_DECREASES(n + 1);
