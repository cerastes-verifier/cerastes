// expect: rejected cerastes-call_without_contract @ L11
// mode: c23, c++20
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 6 (question 130): vlibc_free_aligned_sized has no contract until free_aligned_sized does: a call is call_without_contract
// Pre-registered from record 0050 item 6: rejected call_without_contract at the call
#include <vlibc/stdlib.h>
#include <cerastes.h>

void f(int *p)
{
  vlibc_free_aligned_sized(p, 16, sizeof(int));  // <- here
}
