// expect: rejected wrong_entity_for_attribute @ L9
// mode: c23, c++20
// twin: tests/memory/marked_field_plain.c
// Pins: 0058 item 4 (item 5): must_init on a field stays EDG's own refusal (#1835: the registration has no target d for it)
// Pre-registered from record 0058 item 4: rejected wrong_entity_for_attribute at the field (observed: build/m4.5d2b2d/emul3/r8.c)
#include <cerastes.h>

struct S {
  int *q CERASTES_MUST_INIT;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
