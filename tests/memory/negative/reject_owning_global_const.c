// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 (one pin per rejected shape; the encoder reports it at the declaration carrying the attribute): owning on a pointer to a const object: the block could never be freed without a cast that drops the qualifier (0040 item 2)
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING const int *g = NULL;  // <- here

int main(void)
{
  return 0;
}
