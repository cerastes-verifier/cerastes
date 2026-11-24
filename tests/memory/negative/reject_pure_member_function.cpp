// expect: rejected cerastes-unsupported_declaration @ L10
// expect: rejected cerastes-unsupported_expression @ L19
// expect: frontend-warning set_but_not_used @ L17
// mode: c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 1 (M7): a pure member function is rejected: the struct with a member function is unsupported_declaration (decision 0012) and the call unsupported_expression; measured on the frozen base build, unchanged by this session
// Pre-registered from record 0062 (Part B), before the code: rejected. measured on build/m4.5e1d/cerastes-base: both errors, and EDG's set-but-not-used warning on s
#include <cerastes.h>

struct S {
  int v;
  CERASTES_PURE bool small(int x) const { return x < 10; }
};

int use(int y)
{
  S s;
  s.v = 1;
  CERASTES_CHECK(s.small(y));
  return y;
}
