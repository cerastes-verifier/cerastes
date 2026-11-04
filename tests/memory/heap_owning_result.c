// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 5 (0029 item 1, target r): [[cerastes::owning]] on a function, in its leading and trailing position, is ensures(r == NULL || owns_block(r)); the caller owns the block of a non-null result and frees it
// Pre-registered from record 0047 item 5: verifies in both modes (the flags of the result are set: the callee wrote balance on the non-null path)
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *create_account(void)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a != NULL) {
    a->balance = 0;
  }
  return a;
}

struct account *create_account_trailing(void) CERASTES_OWNING
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a != NULL) {
    a->balance = 0;
  }
  return a;
}

int use_account(void)
    CERASTES_ENSURES(r, r == 5 || r == -1)
{
  struct account *a = create_account();
  if (a == NULL) {
    return -1;
  }
  a->balance = 5;
  int b = a->balance;
  free(a);
  return b;
}

int use_trailing(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct account *a = create_account_trailing();
  if (a == NULL) {
    return -1;
  }
  int b = a->balance;
  free(a);
  return b - b;
}
