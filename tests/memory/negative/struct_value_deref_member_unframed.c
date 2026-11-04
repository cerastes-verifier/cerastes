// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_deref_member.c
// Pins: 0045 item 9: the old rejected shape: *s.q read with no atom on it: permission at the clause
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int f(struct P s)
    CERASTES_REQUIRES(s.q != 0 && *s.q == 1)  // <- here
{
  return s.q == 0;
}
