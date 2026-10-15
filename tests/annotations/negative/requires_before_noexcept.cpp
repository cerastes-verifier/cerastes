// expect: frontend-error exp_semicolon @ L11
// expect: native-ok
// twin: tests/annotations/contracts/noexcept_cv.cpp
// Mutation of the twin: the attribute between cv and noexcept is a syntax error, EDG's own (read from a run on the unregistered namespace, col 40: re-verify after registration); natively the line is `int b() const noexcept`.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct s {
  int v;
  int a() const CERASTES_REQUIRES(v > 0) { return v; }
  int b() const CERASTES_REQUIRES(v > 0) noexcept { return v; }
  int c() & noexcept CERASTES_REQUIRES(v > 0) { return v; }
};
