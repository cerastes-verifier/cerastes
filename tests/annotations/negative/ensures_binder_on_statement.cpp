// expect: frontend-error attr_result_name_not_allowed @ L10
// expect: native-ok
// twin: tests/annotations/statements/check_null.cpp
// Mutation of the twin: a result name outside a function declarator; EDG may also report wrong_entity_for_attribute at the same line (pin on the first run).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int pick(int a, int b) CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  CERASTES_ENSURES(r, a + b >= 0);
  int m;
  if (a < b) {
    CERASTES_CHECK(a < b);
    m = a;
  } else {
    CERASTES_CHECK(a >= b);
    m = b;
  }
  {
    CERASTES_CHECK(m <= a && m <= b);
  }
  return m;
}
