// expect: frontend-error attr_redeclaration_mismatch @ L10
// expect: native-ok
// twin: tests/annotations/contracts/decreases_redecl.cpp
// Mutation of the twin: a redeclaration that adds a second measure.
// Pins: 0006 item 6 for decreases
#include <cerastes.h>


int f(int n) CERASTES_DECREASES(n);
int f(int n) CERASTES_DECREASES(n) CERASTES_DECREASES(n);
