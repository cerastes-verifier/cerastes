// expect: error null-dereference @ L15
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a: the fresh result may be NULL (the contract says so): writing it without the test is null-dereference
// Pre-registered from record 0052 item 2a: null-dereference at the write
#include <cerastes.h>
#include <stddef.h>

int *fresh(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

int use_fresh(void)
{
  int *p = fresh();
  *p = 2;  // <- here
  return 0;
}
