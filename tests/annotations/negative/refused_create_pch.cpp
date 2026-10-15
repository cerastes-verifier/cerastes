// expect: frontend-refused precompiled headers
// cerastes-fe-options: --c++20 --create_pch=x.pch
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: precompiled headers would replay the header without re-running the back end's macro rule.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
