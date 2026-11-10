// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2: cerastes::owning on a field of a union (deferred beyond M4); COLLISION: 0043 item 2 rejects the union itself (reject_struct_union.c: unsupported_declaration at the `union` line), so the encoder may report that line instead of the field
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field (derived from the record, not observed). COLLISION, for the orchestrator: without the attribute 0043 item 2 already rejects this shape with cerastes-unsupported_declaration at the `union` line; which diagnostic wins once the reader lands is the encoder's choice, and a different tag or line is a question-160 correction only if the record agrees
#include <cerastes.h>

union U {
  int *q CERASTES_OWNING;  // <- here
  int n;
};

int ident(int a)
{
  return a;
}
