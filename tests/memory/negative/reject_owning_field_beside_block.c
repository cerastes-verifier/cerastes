// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_replace.c
// Pins: 0055 items 3, 11: a block site on the owning field in any shape other than the closure's, beside an atom that covers it (here with no null alternative), is rejected: owns(*p) already includes the block of owning field p->data
// Pre-registered from record 0055 item 11: rejected unsupported_annotation at the clause (the declaration line, as reject_owns_block_load_beside.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void f(struct buf *p) CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_block(p->data));  // <- here
