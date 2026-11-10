// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 2 (as reject_owning_global_volatile.c): cerastes::owning on a pointer to volatile; COLLISION: 0043 item 2 may refuse the member first (unsupported_declaration at the same line)
// Pre-registered from record 0055 item 2: rejected unsupported_annotation at the field (derived from the record, not observed). COLLISION, for the orchestrator: without the attribute 0043 item 2 already rejects this shape with cerastes-unsupported_declaration at the member; which diagnostic wins once the reader lands is the encoder's choice, and a different tag or line is a question-160 correction only if the record agrees
#include <cerastes.h>

struct S {
  volatile int *q CERASTES_OWNING;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
