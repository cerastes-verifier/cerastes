// expect: rejected cerastes-owner_result_unowned @ L13
// expect: rejected cerastes-owner_release_unowned @ L18
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 ("rejected although harmless"): an allocation stored in a non-owning local and freed in the same function: both rules report, p4 at the malloc and p6 at the free (the default mode verifies it)
// Pre-registered from record 0058 item 3: rejected owner_result_unowned at the call of malloc and owner_release_unowned at the free (two diagnostics in one function: as reject_ref_to_uninit_void_local.c, the encoder goes on after a rejection; a best reading, question 160)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *t = (int *)malloc(sizeof(int));  // <- here
  if (t == NULL) {
    return -1;
  }
  *t = 1;
  free(t);  // <- here
  return 0;
}
