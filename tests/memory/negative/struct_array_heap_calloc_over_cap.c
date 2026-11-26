// expect: rejected cerastes-unsupported_expression @ L18
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_calloc_cap.c
// Pins: 0064 item 15 (question 354): one element past the cap, `int a[24]; int n;` (25 value clauses): calloc is rejected at the call with its own message (it advises malloc and writing each element); malloc and free of the same structure have no cap
// Pre-registered from calloc's typed view writes one value clause per cell leaf and per element of every array leaf, and the cap is 24 clauses (record 0064 item 15, question 354; edg-backend report 6: `int a[23]; int n;` is 24 and accepted, `int a[24]; int n;` is 25 and rejected at the calloc call, `unsupported_expression`, both modes), before any run: rejected cerastes-unsupported_expression at the calloc line (edg-backend report 6, c24: rejected at the call with 25 values, both modes)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct Cap {
  int a[24];
  int n;
};

int over_cap(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct Cap *p = (struct Cap *)calloc(1, sizeof(struct Cap));  // <- here
  if (p == NULL) {
    return -1;
  }
  free(p);
  return 0;
}
