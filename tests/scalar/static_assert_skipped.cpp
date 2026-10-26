// expect: verified
// expect: native-ok
// Pins: 0024 item 3 (owner, handoff Q39): skipped until M5.0 (type traits); EDG evaluated them already
#include <cerastes.h>
#include <concepts>
#include <limits>
#include <type_traits>

static_assert(std::is_integral_v<int>, "a type trait");
static_assert(std::numeric_limits<int>::max() == 2147483647, "a constexpr call into a system header");
static_assert(std::numeric_limits<int>::min() < 0, "another one");
static_assert(std::numeric_limits<int>::digits == 31, "a member of a system class");
static_assert(std::integral<int>, "a concept-id");
static_assert(sizeof(int) == 4 && std::is_integral_v<int>, "the whole assertion is skipped when one operand needs the front end");

int nine(void)
    CERASTES_ENSURES(r, r == 9)
{
  static_assert(std::is_signed_v<int>, "a trait at block scope");
  return 3 * 3;
}
