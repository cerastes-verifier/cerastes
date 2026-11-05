// expect: error permission @ L17
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_borrow_owns.c
// Pins: 0050 items 3, 5: an owning result (owns_block(r), the standard allocator's block) built from vlibc_calloc
// Pre-registered from record 0050 item 5: permission at the return (the value row of the return's token row; one error)
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *create_account(void)
{
  struct account *a = (struct account *)vlibc_calloc(1, sizeof(struct account));
  return a;  // <- here
}
