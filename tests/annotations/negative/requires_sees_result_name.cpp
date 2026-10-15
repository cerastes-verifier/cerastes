// expect: frontend-error undefined_identifier @ L9
// expect: native-ok
// twin: tests/annotations/contracts/requires_ensures_order.cpp
// Mutation of the twin: the result name of an earlier ensures is not in scope in a requires.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int a_first(int x) CERASTES_REQUIRES(x > 0) CERASTES_ENSURES(r, r > x);
int b_first(int x) CERASTES_ENSURES(r, r > x) CERASTES_REQUIRES(r > 0);
