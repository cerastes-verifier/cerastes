// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 7 (audit item 14): the cover disjunction runs over the leaves of the atom's pointee type only (here the one long leaf of a struct with int, long and char leaves): a leaf of another type in it would give the caller a case it cannot frame
// Pre-registered from record 0054 item 7: verifies in both modes (the other-type leaves are not part of the cover, so f__wf stays well formed)
#include <cerastes.h>

struct M {
  int a;
  long b;
  char c;
};

long *where_long(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct M m;
  m.b = 7;
  return &m.b;
}

int main(void)
{
  long *p = where_long();
  *p = 9;
  CERASTES_CHECK(*p == 9);
  return 0;
}
