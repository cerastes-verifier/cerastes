// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 15 (R1): the array parameter `const int a[3]` is a pointer parameter: reads(*a) takes its first element, the caller's local array passes whole; verifies and builds with clang and GCC (the C spelling, both modes)
// Pre-registered from record 0060 item 15 (Part E), before the code: verifies
#include <cerastes.h>

int first(const int a[3])
    CERASTES_REQUIRES(cerastes_reads(*a))
    CERASTES_ENSURES(r, cerastes_reads(*a) && r == cerastes_old(*a))
{
  return a[0];
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x[3] = {7, 8, 9};
  return first(x);
}
