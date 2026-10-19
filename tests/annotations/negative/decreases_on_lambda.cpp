// expect: frontend-error wrong_entity_for_attribute @ L11
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: decreases after a lambda's parameter list.
// Pins: a lambda cannot carry a contract (EDG refuses it, as for requires)
#include <cerastes.h>


int use(int x)
{
  auto f = [](int y) CERASTES_DECREASES(y) { return y; };
  return f(x);
}
