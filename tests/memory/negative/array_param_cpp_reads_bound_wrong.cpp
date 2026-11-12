// expect: error postcondition @ L19
// mode: c++20
// twin: tests/memory/array_param_cpp_reads_bound.cpp
// Pins: 0060 item 15 (R1): the caller claims r == 8 where first returns the array's first element 7: postcondition at the return
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int first(const int a[3])
    CERASTES_REQUIRES(cerastes::reads(*a))
    CERASTES_ENSURES(r, cerastes::reads(*a) && r == cerastes::old(*a))
{
  return a[0];
}

int use(void)
    CERASTES_ENSURES(r, r == 8)
{
  int x[3] = {7, 8, 9};
  return first(x);
}
