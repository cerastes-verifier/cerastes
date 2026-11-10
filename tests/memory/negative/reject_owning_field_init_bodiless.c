// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 items 3, 11 (audit round 2 (f)): a bodiless initializer of a structure with an owning field would hand a block through a load with no requires partner, which nothing verifies: rejected, naming the contract manifest (M6); the bodied initializer verifies (owning_field_lifecycle.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), b_init_bodiless.c: rejected unsupported_annotation at the declaration (a one-line declaration)
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_init(struct buf *p CERASTES_MUST_INIT) CERASTES_REQUIRES(cerastes_owns(*p)) CERASTES_ENSURES(cerastes_owns(*p));  // <- here

int main(void)
{
  struct buf s;
  buf_init(&s);
  free(s.data);
  return 0;
}
