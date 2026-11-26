// expect: error permission @ L21
// expect: error permission @ L31
// mode: c23, c++20
// twin: tests/memory/array_struct_decay.c
// Pins: 0064 item 6 (audit round 1, issue 8): the scope exit exhales exactly what the allocation gave, every leaf path: a pointer to an element stored out of the block and read after it, through q->x and through q->y (the second leaf, so that a leaf path missed at the exit, which would leave its permission behind, shows), is a use after the scope reported as permission (0032 item 6: a read through *q or q->x is permission; only the element access q[0] takes the liveness row)
// Pre-registered from the record's revised item 6 (Part B), before any run: two errors, one per function
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int first(void)
{
  struct P *q = 0;
  {
    struct P v[2] = {{1, 2}, {3, 4}};
    q = &v[1];
  }
  return q->x;  // <- here
}

int second(void)
{
  struct P *q = 0;
  {
    struct P v[2] = {{1, 2}, {3, 4}};
    q = &v[1];
  }
  return q->y;  // <- here
}
