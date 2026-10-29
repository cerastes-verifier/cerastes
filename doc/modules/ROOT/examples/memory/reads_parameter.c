// expect: verified
// expect: native-ok
// mode: c23, c++20
// A parameter that is only read: reads lends a fraction, and the caller can write again after the call.
// tag::source[]
#include <cerastes.h>

int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int read_then_write(void)
    CERASTES_ENSURES(r, r == 8)
{
  int x = 4;
  int a = peek(&x);
  x = a * 2;
  return x;
}
// end::source[]
