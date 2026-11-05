// expect: error permission @ L26
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_borrow_owns.c
// Pins: 0050 items 3, 5 (question 129): a vlibc block passed to a function whose contract says owns_block, which keeps meaning a block of the standard allocator
// Pre-registered from record 0050 item 5: permission at the call (the value row of the call's token row)
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct account {
  int balance;
};

void dispose(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
  free(a);
}

int make_and_dispose(void)
{
  struct account *a = (struct account *)vlibc_calloc(1, sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  dispose(a);  // <- here
  return 0;
}
