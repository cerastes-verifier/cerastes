// expect: frontend-error attr_requires_iteration_statement @ L12
// expect: native-ok
// twin: tests/annotations/statements/decreases_while.cpp
// Mutation of the twin: decreases on an if statement.
// Pins: decision 0013 item 1: a loop annotation, like invariant
#include <cerastes.h>


int pick(int a, int b) CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  int m = a;
  CERASTES_DECREASES(a) if (a < b) {
    m = b;
  }
  return m;
}
