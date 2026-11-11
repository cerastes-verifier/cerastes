// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// twin: tests/memory/strict_owner_both_ref_to_uninit.c
// Pins: 0058 item 3 (audit S6): an owning local that is not marked ref_to_uninit keeps --strict-init's rejection of malloc's result under both options: the owner allowance of --strict-owner must not widen strict-init's set
// Pre-registered from record 0058 item 3 (audit round 1, S6): rejected unsupported_annotation at the call of malloc (the tag --strict-init uses for malloc into a receiver without the marker, as reject_strict_init_malloc_local.c; observed on today's build, emul3/s6a.c)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));  // <- here
  free(v);
  return 0;
}
