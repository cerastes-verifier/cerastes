// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 5 and 15, 0060 item 1, 0061 item 2 (flip B): the lambda form of forall (a lambda written as the first argument of cerastes::forall) stays rejected in verify mode, now by M7 (its message says to write cerastes_forall(f, ...) with a pure function: record 0061 item 2, cut 3); C's forall over a pure function is M4.5e1c's (tests/memory/forall_requires_read.c); the expectation is unchanged, only the message and this comment changed
#include <cerastes.h>
#include <stddef.h>

int first(const int *a, int n)
    CERASTES_REQUIRES(n > 0 && cerastes::forall([](int i, const int *b, int m) { return !(0 <= i && i < m) || b[i] >= 0; }, a, n))
{
  return n;
}
