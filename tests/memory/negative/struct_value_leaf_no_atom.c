// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/struct_value_leaf.c
// Pins: 0045 item 9: return *s.q without an atom on it: permission at the read
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int deref(struct P s) CERASTES_REQUIRES(s.q != 0)
{
  return *s.q;  // <- here
}
