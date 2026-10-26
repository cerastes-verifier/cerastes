// expect: error overflow @ L8
// twin: tests/scalar/static_assert_skipped_gap.cpp
// Pins: 0024 item 3: static_assert_skipped_gap.cpp without the type trait: nothing skips it, so the overflow is reported
#include <cerastes.h>

// The same product as in static_assert_skipped_gap.cpp, without the type trait that made the front end evaluate
// the whole assertion there: nothing skips this one, so the wrap is checked and reported (record 0024 item 3).
static_assert(65536u * 65536u == 0u, "checked");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
