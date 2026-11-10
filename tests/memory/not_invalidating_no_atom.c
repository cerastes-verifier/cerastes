// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 ("No atom rooted at the parameter"): a marked parameter with no atom in the contract is accepted: the check holds vacuously (a bodiless contract: a body that ignores p would only draw an unused-parameter warning), the callee holds nothing through p, so it can neither free nor replace anything p reaches; the caller keeps its own cell
// Pre-registered from record 0058 item 2: verifies in both modes
#include <cerastes.h>

int pass(int *p CERASTES_NOT_INVALIDATING, int n)
    CERASTES_ENSURES(r, r == n);

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int v = pass(&x, 3);
  x = 2;
  return v;
}
