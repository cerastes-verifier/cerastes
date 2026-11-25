// expect: rejected cerastes-unsupported_declaration @ L8
// c-only: array compound literals are C only
// twin: tests/memory/array_literal_decay.c
// Pins: 0063 item 12 (0044 item 7, P67 H5): a file-scope array literal is static storage: rejected as the initializer of a global (the tag is 0044's for a struct global; to confirm at the joined run)
// Pre-registered from record 0063 item 12 (Part D), before any run: rejected
#include <cerastes.h>

static int *gp = (int[]){1, 2, 3};  // <- here

int main(void)
{
  return gp[0] - 1;
}
