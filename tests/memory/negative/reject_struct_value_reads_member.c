// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3 (M4.5c2b): reads(*s.q) of a pointer member of a by-value parameter
#include <cerastes.h>

struct Q {
  int *q;
};

int f(struct Q s)
    CERASTES_REQUIRES(cerastes_reads(*s.q))  // <- here
{
  return s.q == 0;
}
