// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 items 1 and 3: live (like owns, owns_range and owns_block) in a pure function's requires is rejected (M4.5f1): only reads(*p) and reads_range(p, n) are atoms of a pure function
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the atom's line)
#include <cerastes.h>

CERASTES_PURE bool is_there(const int *p)
    CERASTES_REQUIRES(cerastes_live(p))
{
  return p != 0;
}

int use(int y)
{
  return y;
}
