// expect: error permission @ L28
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_owning_platform.c
// Pins: 0050 items 3, 5 (question 129): the same program handing a block of vlibc's heap to the owns_block consumer instead of the platform block
// Pre-registered from record 0050 item 5: permission at the call (the value row of the call's token row), one error
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

int f(void)
{
  struct account *a = (struct account *)calloc(1, sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  struct account *v = (struct account *)vlibc_calloc(1, sizeof(struct account));
  if (v != NULL) {
    dispose(v);  // <- here
  }
  free(a);
  return 0;
}
