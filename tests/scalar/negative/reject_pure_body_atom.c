// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: an atom (reads) in the body of a read pure function is unsupported_annotation (forall_reject_e_atom.c)
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the construct's own line: the function is read by its requires alone)
#include <cerastes.h>


CERASTES_PURE bool g(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return cerastes_reads(*p) && *p > 0;
}

int use(int y)
{
  return y;
}
