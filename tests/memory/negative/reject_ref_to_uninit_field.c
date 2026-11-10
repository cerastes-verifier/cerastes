// expect: rejected wrong_entity_for_attribute @ L9
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0055 item 11 (question 192): ref_to_uninit on a field stays EDG's refusal of the attribute here (#1835, no target d), until d2b2c registers it
// Pre-registered from record 0055 item 11: rejected wrong_entity_for_attribute at the field (0042's registration targets are r|v|p; observed on today's encoder for owning before this record)
#include <cerastes.h>

struct S {
  CERASTES_REF_TO_UNINIT int *q;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
