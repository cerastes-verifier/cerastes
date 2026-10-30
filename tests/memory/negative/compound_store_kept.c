// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/compound_store_address.c
// Pins: 0039 item 5: the callee kept the cell of x (it takes owns(*p) and gives nothing back), so the compound store through *(q = &x) has no permission: permission at the store
#include <cerastes.h>
#include <stddef.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int kept(void)
{
  int x = 1;
  int *q = &x;
  dispose(&x);
  *(q = &x) += 1;
  return *q;
}
