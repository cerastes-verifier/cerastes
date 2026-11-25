// expect: rejected cerastes-unsupported_type @ L14
// expect: rejected cerastes-unsupported_type @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_limit.c
// Pins: 0063 item 13 (question 295, 309, M4.5f3): a structure holding more than 7 array elements (8; the limit is the constant vir::kMaxByValueArrayElements, which the owner may raise) is rejected as a by-value parameter and as a by-value result (`unsupported_type`, at the function's line); a copy has no limit
// Pre-registered from record 0063 item 13 (Part E), before any run: two rejections, at the two function lines (tag from the design; confirm at the first run)
#include <cerastes.h>
#include <stddef.h>

struct B {
  int a[8];
};

int param(struct B b)  // <- here
{
  return b.a[0] * 0;
}

struct B result(void)  // <- here
{
  struct B b = {0};
  return b;
}
