// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 7 ("The working shape": the value facts stand under !cov and the cover frames the read after it): a covered atom with a user read after it, ensures(r, owns(*r) && *r == 1), body sets s.x = 1; the caller learns *p == 1
// Pre-registered from record 0054 item 7 and 0052 item 2 ("Order": the implicit atoms are emitted before the written ones): verifies in both modes
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r) && *r == 1)
{
  static struct S s;
  s.x = 1;
  return &s.x;
}

int main(void)
{
  int *p = where();
  CERASTES_CHECK(*p == 1);
  return *p - 1;
}
