// expect: rejected cerastes-uninit_ref_initialized @ L15
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_global_strict.c
// Pins: 0054 item 5 ("Under --strict-init", 0052 item 4's rule for a marked local): a marked global receiving &x of a local that has an initializer is rejected: "g is marked ref_to_uninit but receives &x, which may point to an initialized object"
// Pre-registered from record 0054 item 5: rejected uninit_ref_initialized at the assignment (the tag of 0052 item 4's marked locals)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *g = NULL;

int main(void)
{
  int x = 0;
  g = &x;  // <- here
  g = NULL;
  return x;
}
