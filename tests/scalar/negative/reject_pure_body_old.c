// expect: rejected cerastes-old_outside_ensures @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3: cerastes_old in the body of a read pure function (a function has one state) is rejected, never read as a call: old_outside_ensures (the tag of cerastes_old outside an ensures, forall_reject_old_in_requires.c)
// Pre-registered from record 0062 (Part B), before the code: rejected. the tag and the line are question-160 candidates (the record names the construct and not the tag: unsupported_annotation is the other candidate)
#include <cerastes.h>


CERASTES_PURE bool g(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return cerastes_old(*p) > 0;
}

int use(int y)
{
  return y;
}
