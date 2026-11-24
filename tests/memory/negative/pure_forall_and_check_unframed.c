// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0062 items 2, 6 and 9: a pure function with no contract that a cerastes_forall names (inlined there) and a check also calls is read; it cannot carry a contract (a forall naming a contracted function is rejected, question 265), so its read of a[i] is unframed: the permission error, in the function, at the read
// Pre-registered from record 0062 (Part B), before the code: one permission error at the read in nonneg (the unframed read takes its default in the Viper function, so the check holds); open to a question-160 correction after its first run
#include <cerastes.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(nonneg, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  CERASTES_CHECK(nonneg(0, a, n));
  (void)a;
  return n;
}
