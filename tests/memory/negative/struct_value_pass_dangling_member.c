// expect: error lifetime @ L23
// mode: c23, c++20
// twin: tests/memory/struct_value_pass_object.c
// Pins: 0044 item 5: passing q by value reads its pointer member, which points into a block that ended: lifetime at the call (a pointer member is read as a use)
#include <cerastes.h>

struct Q {
  int *p;
};

int use(struct Q q)
{
  return 0;
}

int bad(void)
{
  struct Q q = {0};
  {
    int x = 0;
    q.p = &x;
  }
  return use(q);  // <- here
}
