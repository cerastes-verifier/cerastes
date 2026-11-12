// expect: error out-of-bounds @ L16
// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/verifast/carrays.c
// Pins: 0060 item 1 and 0059 item 4 (VeriFast's array_test_fail, L35): a[-1] under owns_range(a, 5) is below the range: out-of-bounds and permission at the store
// Pre-registered from record 0060 item 1 (S43 N1: the bound row and the slot's permission each fail at the store), checked against the frozen Commit 3 build
// Port of VeriFast's examples/carrays.c array_test_fail (~/Documents/verifast at 0fb63506): `//@ requires a[0..5] |-> _` / `ensures a[0..5] |-> _` is `cerastes_owns_range(a, 5)` in both, `a[-1] = 0;` is the store VeriFast marks `should_fail`; `n` is VeriFast's unused parameter, with a `(void)` use.
// Line map (this file : VeriFast's):  11 : 31   16 : 35
#include <cerastes.h>

void array_test_fail(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, 5))
    CERASTES_ENSURES(cerastes_owns_range(a, 5))
{
  (void)n;
  a[-1] = 0;
}
