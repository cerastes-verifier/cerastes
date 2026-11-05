// expect: rejected cerastes-call_without_contract @ L11
// mode: c23, c++20
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 6 (question 130): vlibc_aligned_alloc has no contract until aligned_alloc does: a call is call_without_contract
// Pre-registered from record 0050 item 6: rejected call_without_contract at the call
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  if (vlibc_aligned_alloc(16, 16) != NULL) {  // <- here
    return 1;
  }
  return 0;
}
