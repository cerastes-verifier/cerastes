// expect: rejected cerastes-unsupported_type @ L9
// mode: c++20
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (audit round R, should-fix 1; question 341), the C++ form of restrict_reject_redeclared.c: a function without a body whose first declaration is plain (with the contract) and whose second gives a parameter C++'s __restrict is rejected, as it is with __restrict on the first declaration (audit probe c04, rejected at the declaration, unsupported_type). C++'s tag for the keyword on a parameter is unsupported_type, not item 19's unsupported_annotation (audit note 7a). The audit's probe c10 (build/m4.5e1e2b/audit-R/): verified at the Commit 1 build, because the front end's merged type of the two declarations carries no __restrict and the encoder read only that
// Pre-registered from record 0064 item 19 (question 341), before any run: one row, at the head of the FIRST declaration (edg-backend's round R build, report 6: C++ reports a plain-first, `__restrict`-second function at the first declaration's head, not at the later declaration and not at the call; the record's "at the first call, or at the declaration when read early" does not say which for C++)
#include <cerastes.h>
#include <stddef.h>

void put(int *p)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p) && *p == 5);
void put(int *__restrict p);

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  put(&x);  // <- here
  return x;
}
