// expect: frontend-error wrong_entity_for_attribute @ L9
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: decreases on a variable declaration.
// Pins: decision 0013 item 1: only functions and loops
#include <cerastes.h>


CERASTES_DECREASES(1) int g_value = 0;
