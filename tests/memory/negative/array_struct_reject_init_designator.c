// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23
// c-only: array designators are C (in C++ an array designator is the front end's c99_designator error)
// twin: tests/memory/array_struct_init.c
// Pins: 0064 item 6 (0059 item 3): an array designator over an array of structures is rejected in C as over an array of scalars (array_reject_designator.c: EDG keeps only the folded index outside templates; lifted by M8); the tag is that pin's, to confirm at the joined run
// Pre-registered from record 0064 item 6 (Part B), before any run: rejected (tag by array_reject_designator.c)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int designated(void)
{
  struct P v[2] = {[1] = {1, 2}};  // <- here
  return v[1].x;
}
