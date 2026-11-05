// expect: rejected cerastes-call_without_contract @ L11
// mode: c23, c++20
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 6 (question 130): vlibc_realloc is declared without a contract (realloc's comes with M4.5e2): a call, in the cast form too, is call_without_contract
// Pre-registered from record 0050 item 6: rejected call_without_contract at the call (the cast form (int *)vlibc_realloc(...) was unsupported_type before)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)vlibc_realloc(NULL, sizeof(int));  // <- here
  return p == NULL ? 0 : 1;
}
