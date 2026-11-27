// expect: rejected cerastes-unsupported_annotation @ L25
// expect: rejected cerastes-unsupported_annotation @ L29
// expect: rejected cerastes-unsupported_annotation @ L37
// mode: c23, c++20
// twin: tests/memory/struct_array_aggregate_ok.c
// Pins: 0064 item 15's last clause and question 348 (M4.5e2), the rows that negative/struct_array_reject_aggregate_members.c lacks (audit round D1, finding 5; a second file so that file's line numbers stay as registered): an `owning` global, an `owning` local and a `ref_to_uninit` local whose pointee is struct T { struct P v[2]; int n; }, each rejected at its declaration (the backstops of check.cpp, 632-636, 1037-1043 and the marker row, have no self-test case)
// Pre-registered from the audit round's probe output (audit-D1/d10_owning_global.c, d09_owning_local.c, d14_ref_local.c: `cerastes-unsupported_annotation` at the declaration, both modes; build/m4.5e1e2c/audit-D1/batch5.out), to confirm at the final run: rejected Corrected at the second interim run (edg-backend report 7): `(void)q;` after L37 drew C++20's `set_but_not_used @ L37`, so the use is `q->n = 0;`; no row moved
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct T *make_t(void)
    CERASTES_ENSURES(r, r == NULL);

CERASTES_OWNING struct T *g_owning;  // <- here

void owning_local(void)
{
  CERASTES_OWNING struct T *q = make_t();  // <- here
  (void)q;
}

void ref_to_uninit_local(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  CERASTES_REF_TO_UNINIT struct T *q = p;  // <- here
  q->n = 0;
}
