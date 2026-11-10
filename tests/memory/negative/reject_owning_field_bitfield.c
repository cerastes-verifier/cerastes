// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2: cerastes::owning on a bit-field; COLLISION: 0043 item 2 already rejects a bit-field member (reject_struct_bitfield.c: unsupported_declaration at the member), and a bit-field is never a pointer
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field (derived from the record, not observed). COLLISION, for the orchestrator: without the attribute 0043 item 2 already rejects this shape with cerastes-unsupported_declaration at the member; which diagnostic wins once the reader lands is the encoder's choice, and a different tag or line is a question-160 correction only if the record agrees
#include <cerastes.h>

struct S {
  unsigned b CERASTES_OWNING : 3;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
