// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/marked_global.c
// Pins: 0054 item 5 (one pin per rejected shape; the encoder reports it at the declaration carrying the attribute): ref_to_uninit on a non-pointer global (0042 item 4, never lifted)
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int g = 0;  // <- here

int main(void)
{
  return 0;
}
