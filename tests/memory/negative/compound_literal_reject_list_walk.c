// expect: rejected cerastes-unsequenced_calls @ L23
// expect: rejected cerastes-unsequenced_calls @ L30
// expect: rejected cerastes-unsequenced_calls @ L38
// c-only: compound literals are C (C++ rejects them, record 0044 item 7)
// twin: tests/memory/compound_literal_effects_allowed.c
// Pins: 0063 item 12 (audit round 2, N2; probes r1, r3b, r4): the list rule sees a writing compound literal through a user cast, through designators in order and through nested braces (the elements are flattened), beside another element: rejected like the plain shapes of compound_literal_reject_list_effects.c. The tag is the auditor's observation on k1; the lines are to confirm at the run
// Pre-registered from audit round 2 of record 0063 (N2), before any run: rejected
#include <cerastes.h>

struct P {
  int *p;
  int *q;
};

struct Q {
  struct P in;
  int *r;
};

int through_cast(void)
{
  int x = 1;
  int *a[2] = {(int *)(int[]){x++}, (int[]){x}};  // <- here
  return a[1][0] - 2;
}

int through_designator(void)
{
  int x = 1;
  struct P s = {.p = (int[]){x++}, .q = (int[]){x}};  // <- here
  return s.q[0] - 2;
}

int through_nested_braces(void)
{
  int x = 1;
  int y = 0;
  struct Q q = {{(int[]){x++}, &y}, (int[]){x}};  // <- here
  return q.r[0] - 2;
}
