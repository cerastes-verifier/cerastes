// expect: rejected cerastes-unsupported_annotation @ L38
// expect: rejected cerastes-unsupported_annotation @ L39
// expect: rejected cerastes-unsupported_annotation @ L46
// expect: rejected cerastes-unsupported_annotation @ L47
// expect: rejected cerastes-unsupported_annotation @ L54
// expect: rejected cerastes-unsupported_annotation @ L55
// expect: rejected cerastes-unsupported_annotation @ L62
// expect: rejected cerastes-unsupported_annotation @ L63
// expect: rejected cerastes-unsupported_annotation @ L70
// expect: rejected cerastes-unsupported_annotation @ L71
// expect: rejected cerastes-unsupported_annotation @ L78
// expect: rejected cerastes-unsupported_annotation @ L81
// expect: rejected cerastes-unsupported_annotation @ L84
// expect: rejected cerastes-unsupported_annotation @ L87
// expect: rejected cerastes-unsupported_annotation @ L90
// mode: c23, c++20
// twin: tests/memory/matrix_member_nested.c
// Pins: 0064 item 15's last clause and question 348's seventh route (audit E3a finding 4; edg-backend report 17 section B, the sweep by root kind): a place rooted at a pointer MEMBER of a by-value structure parameter (h.q, struct H { struct V *q; int n; }) or of a by-value result (r.q) is rejected where its structure has an aggregate array: owns(*h.q), reads(*h.q), owns and reads of its member u.m, and a read of its cell under reads(*h.q) in the requires and the ensures of a parameter (two rows), in the ensures of a result (one row); The structure is the nested one (struct V { struct U u; int k; }, the matrix standing one level down); The same route for a structure with an array member of scalars (struct P) is accepted (Part D), a pointer parameter's own atoms (owns(*p)) stay accepted; the companion of the existing position file for this structure, in a file of its own so that no row of that file moves. M4.5e2
// Pre-registered (test-mutator-10, Task 1/2, before any run): rows registered from report 17 section B's table (every row an encoder rejection, `unsupported_annotation`, at the contract line, both modes; the five atoms of the sweep, the byval and resultbyval forms), the lines mapped to this file's; measured in the combined form on the final join; corrected once after the first probe in lane edg (question 160, program only, no expect line moves): the five by-value functions had `(void)h;` in the body, which is an `unsupported_expression` row of its own at that line (the encoder's existing rule: "the struct `h` used as a value (a struct is reached through its members, `&s` and whole-struct copies; any other use of a struct as a value is not supported"), nothing to do with the seventh route, so the statement became a comment on the same line (read from the probe's diagnostic, r7_void.c)
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

struct V {
  struct U u;
  int k;
};

struct H {
  struct V *q;
  int n;
};

int by_owns(struct H h)
    CERASTES_REQUIRES(cerastes_owns(*h.q))  // <- here
    CERASTES_ENSURES(r, cerastes_owns(*h.q))  // <- here
{
  // h is not named here: `(void)h;` is an expression row of its own (the first probe, question 160)
  return 0;
}

int by_reads(struct H h)
    CERASTES_REQUIRES(cerastes_reads(*h.q))  // <- here
    CERASTES_ENSURES(r, cerastes_reads(*h.q))  // <- here
{
  // h is not named here: `(void)h;` is an expression row of its own (the first probe, question 160)
  return 0;
}

int by_owns_member(struct H h)
    CERASTES_REQUIRES(cerastes_owns(h.q->u.m))  // <- here
    CERASTES_ENSURES(r, cerastes_owns(h.q->u.m))  // <- here
{
  // h is not named here: `(void)h;` is an expression row of its own (the first probe, question 160)
  return 0;
}

int by_reads_member(struct H h)
    CERASTES_REQUIRES(cerastes_reads(h.q->u.m))  // <- here
    CERASTES_ENSURES(r, cerastes_reads(h.q->u.m))  // <- here
{
  // h is not named here: `(void)h;` is an expression row of its own (the first probe, question 160)
  return 0;
}

int by_read(struct H h)
    CERASTES_REQUIRES(cerastes_reads(*h.q) && h.q->u.m[1][2] >= 0)  // <- here
    CERASTES_ENSURES(r, cerastes_reads(*h.q) && h.q->u.m[1][2] >= 0)  // <- here
{
  // h is not named here: `(void)h;` is an expression row of its own (the first probe, question 160)
  return 0;
}

struct H make_owns(void)
    CERASTES_ENSURES(r, cerastes_owns(*r.q));  // <- here

struct H make_reads(void)
    CERASTES_ENSURES(r, cerastes_reads(*r.q));  // <- here

struct H make_owns_member(void)
    CERASTES_ENSURES(r, cerastes_owns(r.q->u.m));  // <- here

struct H make_reads_member(void)
    CERASTES_ENSURES(r, cerastes_reads(r.q->u.m));  // <- here

struct H make_read(void)
    CERASTES_ENSURES(r, cerastes_reads(*r.q) && r.q->u.m[1][2] >= 0);  // <- here
