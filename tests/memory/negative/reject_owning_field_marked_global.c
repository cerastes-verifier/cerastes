// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 items 3, 11 (audit round 2): ref_to_uninit on an owning global whose pointee structure has an owning field is rejected like the parameter and result (M4.5d2b2c): an object with an owning field may not be marked as holding no value (n_marked_global.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), n_marked_global.c and the edg-backend's fix in progress: rejected unsupported_annotation at the declaration; the program is only the declaration and an empty main, so that the pin is one error and nothing that depends on the marked global; the first fix gave the declaration's error plus two errors at owns(gp) in a contract, which the pin no longer has
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING CERASTES_REF_TO_UNINIT;  // <- here

int main(void)
{
  return 0;
}
