// expect: rejected cerastes-unsupported_annotation @ L48
// expect: rejected cerastes-unsupported_annotation @ L59
// expect: rejected cerastes-unsupported_annotation @ L71
// expect: rejected cerastes-unsupported_annotation @ L83
// expect: rejected cerastes-unsupported_annotation @ L94
// expect: rejected cerastes-unsupported_annotation @ L105
// expect: rejected cerastes-unsupported_annotation @ L117
// expect: rejected cerastes-unsupported_annotation @ L133
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_moved_struct.c
// Pins: 0064 item 15 (Part D, step B; audit round D2 finding 6, edg-backend report 10): the written atoms in a loop invariant on a structure with an array member or on an array member that stay rejected, one row each at the invariant's line: written reads(*p) (x8) and reads(p->a) (x9); a pointer read through memory owns(*r->q) (x1); a global pointer owns(*gp) (x7); an address-taken local owns(*p) (x5); a member of a by-value member owns(w->s.a) (x3); a guard that reads memory (g1: r->z > 0 ? owns(*c) : true); owns(c->a) on a scalar array member of a structure that also has an array member of structures (q7, question 348). The accepted neighbours are in struct_array_loop_moved_struct.c and struct_array_loop_written_neighbours.c
// Pre-registered from edg-backend report 10 (the encoder's new rejections, probes audit-D2/x02b_refused.c, g01_guard_reads.c, q02_348_sibling.c), before any run of this file as a test: eight rejected rows, tag unsupported_annotation; the guard row (c) comes from a later pass of the encoder than the others and the row may need a file of its own (to confirm at the run)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct R {
  struct S *q;
  int z;
};

struct W {
  int z;
  struct S s;
};

struct E {
  int x;
  int y;
};

struct SB {
  int a[4];
  struct E v[2];
};

struct S *gp;

void x1(struct R *r)
    CERASTES_REQUIRES(cerastes_owns(*r) && cerastes_owns(*r->q))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*r->q))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
}

void x3(struct W *w)
    CERASTES_REQUIRES(cerastes_owns(*w))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(w->s.a))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
}

void x5(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  struct S **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*p))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    (*pp)->n = 1;
    i = i + 1;
  }
}

void x7(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*gp))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
}

void x8(const struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_reads(*p))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
}

void x9(const struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_reads(p->a))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
}

void g1(struct S *p, struct S *q, struct R *r)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && cerastes_reads(*r) && r->z > 0)
{
  struct S *c = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && r->z > 0 && (r->z > 0 ? cerastes_owns(*c) : true))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    c->n = 1;
    c->a[1] = 1;
    c = (i < 2) ? p : q;
    i = i + 1;
  }
}

void q7(struct SB *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  struct SB *c = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(c->a))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    c->a[1] = 3;
    c = p;
    i = i + 1;
  }
}
