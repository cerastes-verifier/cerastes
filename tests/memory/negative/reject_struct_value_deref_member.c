// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3 (M4.5c2b): *s.q reads through a pointer member of a by-value parameter: a place reached through a heap read
#include <cerastes.h>

struct Q {
  int *q;
};

int f(struct Q s)
    CERASTES_REQUIRES(s.q != 0 && *s.q == 1)  // <- here
{
  return s.q == 0;
}
