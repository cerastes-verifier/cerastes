// expect: frontend-refused the cerastes back end did not run
// cerastes-fe-options: --c++20 -n
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: -n is EDG's short form of --no_code_gen.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
