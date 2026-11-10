// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 5 (0054 item 10; as reject_owning_global_nonnull_init.c): an owning leaf of a struct global starts null and owns only what it is given: a non-null initializer (here the address of another global) is rejected, "an owning field starts null; it owns only what it is given"
// Pre-registered from record 0055 item 11: rejected unsupported_annotation at the declaration (the line of the global; part B builds it)
#include <cerastes.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

static int h;
struct buf g = {&h, 0};  // <- here

int ident(int a)
{
  return a;
}
