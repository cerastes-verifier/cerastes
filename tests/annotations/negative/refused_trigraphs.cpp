// expect: frontend-refused --trigraphs is not supported
// cerastes-fe-options: --c++20 --trigraphs
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: trigraph replacement would rewrite the source after the scan for a changed __CERASTES__ ran.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
