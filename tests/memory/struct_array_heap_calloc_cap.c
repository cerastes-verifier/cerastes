// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (the cap on calloc's per-element clauses, question 354): the largest accepted structure, `int a[23]; int n;` (24 value clauses), allocated with calloc, one element and n checked to be 0, freed. TIMED at its first run (the cap was set at about half the count that verifies through the driver)
// Pre-registered from calloc's typed view writes one value clause per cell leaf and per element of every array leaf, and the cap is 24 clauses (record 0064 item 15, question 354; edg-backend report 6: `int a[23]; int n;` is 24 and accepted, `int a[24]; int n;` is 25 and rejected at the calloc call, `unsupported_expression`, both modes), before any run: verifies in both modes (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct Cap {
  int a[23];
  int n;
};

int cap_zero(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct Cap *p = (struct Cap *)calloc(1, sizeof(struct Cap));
  if (p == NULL) {
    return -1;
  }
  CERASTES_CHECK(p->a[22] == 0 && p->n == 0);
  free(p);
  return 0;
}
