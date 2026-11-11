// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/return_one_block.c
// Pins: 0058 item 2 (audit round 3, the il-analyst's report 8): B1's reorder of the return rows keeps the same-block row for two formal sites: the contract hands back p's block (ensures owns_block(p)) and gives the result's block (ensures r owns_block(r)), and the body returns p, so both sites name one block: permission at the return, "may be the same object" (owning_field_copy_ptr_both.c pins the same row for two fields)
// Pre-registered from audit round 3 (build/m4.5d2b2d/audit4/ret_same_block.c, report 8): one permission error at the return, both before and after B1
#include <cerastes.h>

int *both(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, cerastes_owns_block(r))
{
  return p;  // <- here
}
