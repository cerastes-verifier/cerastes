// expect: verified
// expect: native-ok
// Pins: 0054 item 2: cerastes::owns_block(*pp) is the C++ spelling of cerastes_owns_block(*pp) through a load (a given site)
// Pre-registered from record 0054 item 2 (forms: "cerastes::owns_block(E)"): verifies
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes::owns(*pp))
    CERASTES_ENSURES(cerastes::owns(*pp) && cerastes::owns_block(*pp) && **pp == 5)
{
  *pp = static_cast<int *>(malloc(sizeof(int)));
  if (*pp == nullptr) {
    abort();
  }
  **pp = 5;
}

int use_make()
    CERASTES_ENSURES(r, r == 5)
{
  int *p = nullptr;
  make(&p);
  int v = *p;
  free(p);
  return v;
}
