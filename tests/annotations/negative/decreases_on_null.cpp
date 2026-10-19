// expect: frontend-error attr_requires_iteration_statement @ L11
// expect: native-ok
// twin: tests/annotations/statements/decreases_while.cpp
// Mutation of the twin: decreases on a null statement.
// Pins: decision 0013 item 1: a loop annotation, like invariant
#include <cerastes.h>


int pick(int a) CERASTES_REQUIRES(a >= 0)
{
  CERASTES_DECREASES(a);
  return a;
}
