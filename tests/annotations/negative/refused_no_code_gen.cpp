// expect: frontend-refused the cerastes back end did not run
// cerastes-fe-options: --c++20 --no_code_gen
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: --no_code_gen would skip the back end and check nothing.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
