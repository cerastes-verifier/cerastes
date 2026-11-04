// expect: rejected cerastes-call_without_contract @ L11
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 1: aligned_alloc has no contract: a call is call_without_contract
// Pre-registered from record 0047 item 1: rejected call_without_contract at the marked line (item 1 names the tag)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  (void)aligned_alloc(16, 16);  // <- here
  return 0;
}
