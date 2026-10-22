// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c++20
// twin: tests/scalar/enum_switch_scoped.cpp
// Pins: 0019 item 11 rule 2 (C++): a scoped enumeration without a default
#include <cerastes.h>

enum class S { SA, SB };

int classify(S s)
{
  switch (s) {
  case S::SA:
    return 1;
  }
  return 0;
}
