// expect: error lifetime @ L13
// mode: c23, c++20
// twin: tests/memory/struct_value_live_member.c
// Pins: 0045 item 9: without live(s.q) the pointer member is not usable: lifetime at the return
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int *f(struct P s)
{
  return s.q;  // <- here
}
