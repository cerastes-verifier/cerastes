// expect: rejected cerastes-unsupported_annotation @ L12
// expect: rejected cerastes-unsupported_type @ L13
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3 (permanent, with 0047 item 11): cerastes::owning on a void* local: a void* is held only through the converted result of an allocation and free's argument
// Pre-registered from record 0052 item 3: rejected unsupported_annotation at the declaration, and the use of the void* is the existing unsupported_type (any use of p is one: (void)p is no use for the C++ front end, which then warns set-but-unused)
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  CERASTES_OWNING void *p = NULL;  // <- here
  return p == NULL ? 0 : 1;  // <- here
}
