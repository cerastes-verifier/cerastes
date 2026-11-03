// expect: error lifetime @ L15
// mode: c23, c++20
// twin: tests/memory/struct_pointer_ops.c
// Pins: 0043 item 8, 0028 item 3a: a comparison uses p's value: a function that holds no live(p) may not, whatever p points to
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

int is_null(struct S *p)
{
  return p == NULL;  // <- here
}
