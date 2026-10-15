// expect: frontend-error cerastes-modules_unsupported @ L7
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a C++20 module unit: EDG parses it in the cerastes configuration, but contracts do not survive module files (invariant 3); not native-ok (the ordinary compilers need -fmodules). `import m;` needs a module file, so EDG stops first with a catastrophic error and cannot be a one-file test.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
module;
#include <cerastes.h>
export module m;

int half(int x) CERASTES_REQUIRES(x >= 0);
