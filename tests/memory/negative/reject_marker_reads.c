// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 4 (question 88): reads(*p) beside a marker: a borrowed read of an object that may hold no value (never lifted)
#include <cerastes.h>

int f(int *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  return 0;
}
