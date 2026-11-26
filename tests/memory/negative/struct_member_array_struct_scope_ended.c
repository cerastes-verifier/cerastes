// expect: error permission @ L27
// expect: error lifetime @ L37
// expect: error permission @ L47
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_scope.c
// Pins: 0064 items 6 and 10 (audit round C2, finding 3): a pointer into the member array of a structure local (the address of a leaf, the decay of the array, the address of an element) read after the block of the structure has ended: the read of `*q` and of `q->y` is a `permission` (the cell may have ended), the use of the pointer q itself after the decay is a `lifetime`; the tag follows the use as the audit measured it (g05: use, use2, use3). One reason: the structure's block has ended
// Registered from audit-C2 probe g05 (use L4 permission, use2 L5 lifetime, use3 L6 permission, measured on snapshot a044aff629b8); the probes' bodies are laid out as this file and the rows' lines inferred (the use of the pointer after the block)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int use(void)
{
  int *q;
  {
    struct T t = {{{1, 2}, {3, 4}}, 5};
    q = &t.v[1].x;
  }
  return *q;  // <- here
}

int use2(void)
{
  struct P *q;
  {
    struct T t = {{{1, 2}, {3, 4}}, 5};
    q = t.v;
  }
  return q[1].x;  // <- here
}

int use3(void)
{
  struct P *q;
  {
    struct T t = {{{1, 2}, {3, 4}}, 5};
    q = &t.v[1];
  }
  return q->y;  // <- here
}
