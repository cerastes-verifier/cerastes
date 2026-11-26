// expect: error check @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_calloc_cap.c
// Pins: 0064 item 15 (question 354): at the cap, after calloc of `int a[23]; int n;` a check that the LAST element is 1 is false and is reported at the check; the view's zero clauses say 0 for every index, the last included
// Pre-registered from calloc's typed view writes one value clause per cell leaf and per element of every array leaf, and the cap is 24 clauses (record 0064 item 15, question 354; edg-backend report 6: `int a[23]; int n;` is 24 and accepted, `int a[24]; int n;` is 25 and rejected at the calloc call, `unsupported_expression`, both modes), before any run: error check at the check (the precedent negative/struct_array_heap_calloc_wrong.c)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct Cap {
  int a[23];
  int n;
};

int cap_wrong(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct Cap *p = (struct Cap *)calloc(1, sizeof(struct Cap));
  if (p == NULL) {
    return -1;
  }
  CERASTES_CHECK(p->a[22] == 1);  // <- here
  free(p);
  return 0;
}
