// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 10 (item 5 "Initializer"): an owning global starts null and owns only what it is given: a non-null initializer (here the address of another global) is rejected, "an owning global starts null; it owns only what it is given"
// Pre-registered from record 0054 item 10: rejected unsupported_annotation at the declaration (it names M4.5d2b2a; the line is the declaration carrying the attribute)
#include <cerastes.h>
#include <stdlib.h>

static int x;
CERASTES_OWNING static int *g = &x;  // <- here

int main(void)
{
  return g == NULL;
}
