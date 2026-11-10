// expect: rejected cerastes-unsupported_annotation @ L17
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 5: the same for a static struct local: an owning leaf starts null
// Pre-registered from record 0055 item 11: rejected unsupported_annotation at the declaration (the line of the static local; part B builds it)
#include <cerastes.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

static int h;

int f(void)
{
  static struct buf s = {&h, 0};  // <- here
  return s.n;
}

int ident(int a)
{
  return a;
}
