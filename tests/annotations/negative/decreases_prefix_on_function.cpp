// expect: frontend-error cerastes-contract_not_after_parameters @ L9
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: decreases written as a prefix of the declaration: it is a contract and goes after the parameter list.
// Pins: decision 0013 item 1: placement like requires (cerastes-contract_not_after_parameters)
#include <cerastes.h>


CERASTES_DECREASES(true) int half(int x);
