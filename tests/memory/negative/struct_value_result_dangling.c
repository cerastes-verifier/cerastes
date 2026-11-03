// expect: error lifetime @ L21
// mode: c23, c++20
// twin: tests/memory/struct_value_result_pointer.c
// Pins: 0044 item 4: n.next = &n; return n; is not R14's shape (the pointer is in a member); it dangles at the caller, whose copy of the result is the first use: lifetime
#include <cerastes.h>

struct N {
  struct N *next;
  int v;
};

struct N mk(void)
{
  struct N n = {0, 1};
  n.next = &n;
  return n;
}

int use(void)
{
  struct N m = mk();  // <- here
  struct N *p = m.next;
  return p == 0;
}
