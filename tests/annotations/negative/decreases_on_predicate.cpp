// expect: frontend-error cerastes-contract_on_predicate @ L9
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_pure.cpp
// Mutation of the twin: a measure on a predicate (a predicate has no contract).
// Pins: decision 0013 item 1: decreases on a function is a contract
#include <cerastes.h>


CERASTES_PREDICATE bool nonneg(int x) CERASTES_DECREASES(x)
{
  return x >= 0;
}
