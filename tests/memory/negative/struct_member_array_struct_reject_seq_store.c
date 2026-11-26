// expect: rejected cerastes-unsequenced_modification @ L21
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_copy.c
// Pins: 0064 item 11 (audit round 2, issue 8): t = (struct T){{{t.v[0].x++, 2}, {3, 4}}, 5}: the store of a structure value writes every leaf of t, among them the member v[0].x of the first element, which the literal's element expression writes too (the static conflict it is for a scalar member, probe sq2.c; in C++ over-strict as for item 5's rule of 0044, not undefined); struct_store_leaves carries the index steps of an array member of aggregates. A CALL as the stored value, t = mkt(t.v[0].x++), is well defined (a call is a sequence point, record 0021) and verifies: it is store_own in the positive
// Pre-registered from record 0064 item 11 (Part C2), before any run: rejected (the tag by negative/reject_struct_value_seq_literal.c's row; CORRECTED once before the first harness run, from the joined proof: the call form t = mkt(t.v[0].x++) of the first draft verifies, the literal draws the rejection, edg-backend's probe sq2.c)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int f(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  t = (struct T){{{t.v[0].x++, 2}, {3, 4}}, 5};  // <- here
  return t.n;
}
