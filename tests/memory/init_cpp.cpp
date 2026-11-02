// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0042 items 2, 4: the markers with the C++ spellings, in the trailing and the leading position, a declaration and a definition both marked
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p) && *p == 0);

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p) && *p == 0)
{
  *p = 0;
}

int fill(CERASTES_REF_TO_UNINIT int *p)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(r, cerastes::owns(*p) && r == 3)
{
  *p = 3;
  return *p;
}

int use_both()
    CERASTES_ENSURES(r, r == 3)
{
  int a;
  init(&a);
  int b;
  int c = fill(&b);
  return a + c;
}
