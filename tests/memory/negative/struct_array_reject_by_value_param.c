// (L11, the member's declaration, is no longer rejected: Part C accepts the structure; question 160 re-pointing, listed in test-mutator-report-8)
// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: record 0063 (M4.5e1e1 staging pin, retired by Part E, Flips A): a by-value parameter of a structure with an array member, reported at the function's line (Part C accepts the structure itself, so the member's declaration is no longer reported; Part E lifts this)
// Pre-registered from the frozen base build's diagnostics (build/m4.5e1e1/s0/results), not guessed; after Part C's re-pointing the remaining line (the function's, L15) is from edg-backend-3's Mac probe r2 and the encoder plan (signature_vtype)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(struct S s)
{
  return s.n;
}
