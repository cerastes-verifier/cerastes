// expect: frontend-error cerastes-old_outside_ensures @ L8
// expect: native-ok
// twin: tests/annotations/contracts/ensures_reference.cpp
// Mutation of the twin: old() in the precondition of a function with a reference parameter.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

void incr(int& x) CERASTES_REQUIRES(cerastes::old(x) < 1000) CERASTES_ENSURES(x == cerastes::old(x) + 1)
{
  x += 1;
}

void incr_ptr(int* p) CERASTES_REQUIRES(*p < 1000) CERASTES_ENSURES(*p == cerastes::old(*p) + 1)
{
  *p += 1;
}
