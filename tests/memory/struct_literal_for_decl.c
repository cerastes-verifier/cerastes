// expect: verified
// expect: native-ok
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// Pins: 0044 item 7 (audit S1, P67 H4): a literal in the declaration of a for init (for (int *p = &(int){1}; ...)) belongs to the block C wraps the for in; p is a variable of that block
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int in_for_decl(void)
    CERASTES_ENSURES(r, r == 3)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  for (int *p = &(int){1}; s < 3; s += 1) {
    *p = s;
  }
  return s;
}
