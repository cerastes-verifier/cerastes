// expect: verified
// expect: native-ok
// Pins: 0024 item 3: the gap, as a verified test, so a change to the skip rule has to change this expectation on purpose (owner review)
#include <cerastes.h>
#include <type_traits>

// A top-level && with one operand the front end must evaluate: the whole assertion is skipped, so the wrapping
// product in its second operand is not checked (the documented gap of record 0024 item 3).
static_assert(std::is_integral_v<int> && 65536u * 65536u == 0u, "skipped as a whole");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
