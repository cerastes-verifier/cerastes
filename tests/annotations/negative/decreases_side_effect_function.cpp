// expect: frontend-error cerastes-annotation_side_effect @ L9
// expect: native-ok
// twin: tests/annotations/contracts/decreases_function.cpp
// Mutation of the twin: the measure decrements a parameter.
// Pins: 0006 item 10: an annotation has no side effects (also decreases)
#include <cerastes.h>


int f(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(n--);
