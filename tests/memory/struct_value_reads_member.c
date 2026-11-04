// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 9 (flip of reject_struct_value_reads_member.c): reads(*s.q) of a by-value parameter's pointer member
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int f(struct P s)
    CERASTES_REQUIRES(cerastes_reads(*s.q))
    CERASTES_ENSURES(r, cerastes_reads(*s.q) && r == *s.q)
{
  return *s.q;
}
