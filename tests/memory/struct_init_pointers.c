// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 2, 3, 6: a pointer member is a leaf cell of a pointer field; {} leaves it null; the pointer is compared, never dereferenced through the member (a contract may follow it since M4.5c2b: tests/memory/struct_chain_contract.c)
#include <cerastes.h>
#include <stddef.h>

struct L {
  int *p;
  int v;
};

int pointer_member(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = 1;
  struct L l = {&a, 2};
  CERASTES_CHECK(l.p == &a);
  struct L m = {};
  CERASTES_CHECK(m.p == NULL && m.v == 0);
  return l.v + m.v;
}
