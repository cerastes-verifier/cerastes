// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 2: A and B share a value, A covers both, C has no case
#include <cerastes.h>

enum dup { A = 0, B = 0, C = 2 };

int classify(enum dup d)
{
  switch (d) {
  case A:
    return 1;
  }
  return 0;
}
