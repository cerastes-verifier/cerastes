// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/marked_field_plain.c
// Pins: 0058 item 4: ref_to_uninit on a field twice is rejected as owning on a field is (0055 item 2): unsupported_annotation at the member
// Pre-registered from record 0058 item 4: rejected unsupported_annotation at the field (observed on the Part F build: build/m4.5d2b2d/emul3/r*.c)
#include <cerastes.h>

struct S {
  int *q CERASTES_REF_TO_UNINIT CERASTES_REF_TO_UNINIT;  // <- here
  int k;
};

int ident(int a)
{
  return a;
}
