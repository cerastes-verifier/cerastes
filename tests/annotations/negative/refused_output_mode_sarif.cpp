// expect: frontend-refused --output_mode=sarif is not supported yet
// cerastes-fe-options: --c++20 --output_mode=sarif
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: cerastes diagnostics cannot be written as SARIF yet.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
