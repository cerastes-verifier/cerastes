// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 item 4: &param copies the parameter into a one-slot block at entry; contracts keep reading the entry value
#include <cerastes.h>
#include <stddef.h>

int set_one(int a)
    CERASTES_REQUIRES(a < 100)
    CERASTES_ENSURES(r, r == 1)
{
  int *q = &a;
  *q = 1;
  return a;
}

int read_back(int a)
    CERASTES_ENSURES(r, r == a)
{
  int *q = &a;
  return *q;
}
