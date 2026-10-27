// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 6 R11: an overlapping comparison inside an annotation is not rejected (code only); logic_and_post.c, static_assert_block_excluded.c and switch_case_requires_false_shift.c rely on it
#include <cerastes.h>

// 0025 item 6 R11 applies to code only: the compilers never see a contract, and a postcondition that always holds is a contract
// (possibly a wrong one), so it is accepted
int always(int a)
    CERASTES_ENSURES(r, r == (a > 0 || a < 10))
{
  (void)a;
  return 1;
}
