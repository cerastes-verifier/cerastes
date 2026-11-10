// expect: error leak @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_take_named.c
// Pins: 0055 item 5 (N3): taking the block into a plain local and then setting the named leaf to NULL is a leak at the assignment: a named leaf follows P4417R0 p3 (an owner-to-non-owner copy does not take the obligation), "assigning to owning field `s.data` drops the block it holds"; through a pointer the same program is accepted (owning_field_take_pointer.c)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at the store to s.data (new: no base form; S40 (d) own_local_take.c is the owning-local precedent)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  *s.data = 1;
  int *t = s.data;
  s.data = NULL;  // <- here
  free(t);
  return 0;
}
