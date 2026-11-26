// expect: rejected cerastes-unsupported_expression @ L15
// mode: c23, c++20
// twin: tests/memory/range_struct_owns.c
// Pins: 0064 item 7 (Part C1, the audit's r15; question 292, a heap block beside a range over structures is Part D's): an `owns_block` atom beside a range over structures in one function, the block holding int cells like the range's leaf fields: one rejection, at the `owns_block` atom, once (the check reports each atom once however many range atoms share a leaf type with it; the `ensures` does not repeat the atom)
// Registered from record 0064 item 7 and il-analyst-report-9 item 3 (the audit's r15 gave two identical rows on one line; the check now reports one row per `owns_block` atom); the tag and the single row were measured by `build/cerastes verify` on the Mac build (a requires-only atom gives one `unsupported_expression` row), not yet on the box: rejected unsupported_expression at the requires line
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void block_beside(struct P *p, int n, int *b)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_block(b))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
  (void)b;
}
