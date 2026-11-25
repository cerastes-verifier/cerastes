// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/array_struct_decay.c
// Pins: 0064 items 4 and 5: q = v is stored out of the block, the block ended: q[0].x takes the liveness row of the element access (lifetime, as array_literal_decay_after_block.c has it for q[0]); the category is the liveness row's, to confirm at the joined run
// Pre-registered from record 0064 items 4 and 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int decay(void)
{
  struct P *q = 0;
  {
    struct P v[2] = {{1, 2}, {3, 4}};
    q = v;
  }
  return q[0].x;  // <- here
}
