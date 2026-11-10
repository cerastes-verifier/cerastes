// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_guarded_global.c
// Pins: 0055 item 5: on the owning arm the store g.data = NULL drops the block the guarded atom gave: the reassignment row (ge_guard_store.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): ge_guard_store.c: the genuine leak, expected at the store (the position is the record's reassignment row; to be confirmed by the run)
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void f(bool c)
    CERASTES_REQUIRES(c ? cerastes_owns(g) : true)
    CERASTES_ENSURES(c ? cerastes_owns(g) : true)
{
  if (c) {
    g.data = NULL;  // <- here
  }
}
