// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/owning_field_replace.c
// Pins: 0055 items 3, 11 (audit round 2 (g)): reads of the field's pointee beside owns(*p), which already includes the block of owning field p->data, is another shape of site on the field and is rejected (c_beside_reads.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), c_beside_reads.c: rejected unsupported_annotation at the clause (a one-line declaration)
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void f(struct buf *p) CERASTES_REQUIRES(cerastes_owns(*p) && (p->data == NULL || cerastes_reads(*p->data))) CERASTES_ENSURES(cerastes_owns(*p)) { }  // <- here
