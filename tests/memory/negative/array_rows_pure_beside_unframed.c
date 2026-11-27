// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/array_rows_forall_beside.c
// Pins: 0064 item 7 (audit round E1, findings 2 and 4; the auditor's j79, an exit 3 on the audited snapshot: the lowering has no row skip in apply_frame, fixed by the il-analyst): a pure application get(q), whose precondition reads *q, beside a range over rows that does not frame q: `permission` at the requires (the application's precondition is not framed), as the same program over a range of structures is (range_struct_pure_beside_unframed.c). The line is the requires; to confirm at the first run
// Pre-registered from record 0064 item 7 (Part E), before any run: one permission error at the requires
#include <cerastes.h>

CERASTES_PURE int get(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

int f([[maybe_unused]] int a[][3], [[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && get(q) == 0)  // <- here
    CERASTES_ENSURES(r, cerastes_owns_range(a, 2))
{
  return 0;
}
