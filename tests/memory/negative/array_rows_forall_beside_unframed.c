// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/array_rows_forall_beside.c
// Pins: 0064 item 7 (audit round E1, findings 2 and 4; the auditor's j77): the quantifier reads q's elements but nothing in the contract owns or reads them, only the range over rows does: the contract of f reads p[i] without owning it (`permission`, at the requires). A range over rows must not frame a read through a different pointer
// Pre-registered from record 0064 item 7 (Part E), before any run: one permission error at the requires
#include <cerastes.h>

CERASTES_PURE bool z(int i, int *p)
{
  return i < 0 || i >= 3 || p[i] == 0;
}

int f([[maybe_unused]] int a[][3], [[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && cerastes_forall(z, q))  // <- here
    CERASTES_ENSURES(r, cerastes_owns_range(a, 2))
{
  return 0;
}
