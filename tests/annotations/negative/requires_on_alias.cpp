// expect: frontend-error wrong_entity_for_attribute @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a contract in an alias-declaration (CP2a probe p12: must not attach to an unnamed typedef).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

using half = int(int x) CERASTES_REQUIRES(x >= 0);
