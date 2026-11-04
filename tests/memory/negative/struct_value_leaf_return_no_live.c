// expect: error lifetime @ L13
// mode: c23, c++20
// twin: tests/memory/struct_value_leaf.c
// Pins: 0045 item 9 (the documented c2a shape): return s.q without live(s.q): lifetime at the return
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int *ident(struct P s)
{
  return s.q;  // <- here
}
