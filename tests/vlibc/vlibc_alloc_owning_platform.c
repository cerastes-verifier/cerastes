// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 items 3, 5 (audit round 2 S3): in a vlibc heap program, a platform block still passes through an owns_block consumer and an owning function's return (the value rows hold for it)
// Pre-registered from record 0050 item 5: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *create_account(void)
{
  struct account *a = (struct account *)calloc(1, sizeof(struct account));
  return a;
}

void dispose(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
  free(a);
}

int f(void)
{
  struct account *a = create_account();
  if (a == NULL) {
    return -1;
  }
  int *v = (int *)vlibc_malloc(sizeof(int));
  if (v != NULL) {
    *v = 1;
    vlibc_free(v);
  }
  dispose(a);
  return 0;
}
