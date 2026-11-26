// expect: rejected cerastes-unsupported_annotation @ L32
// expect: rejected cerastes-unsupported_annotation @ L41
// expect: rejected cerastes-unsupported_annotation @ L49
// expect: rejected cerastes-unsupported_annotation @ L57
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2); the C++ spelling is restrict_reject_redeclared_cpp.cpp
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (audit round R, should-fix 1; question 341): a function without a body is rejected when ANY of its declarations gives a parameter the qualifier, whatever the order and whatever the other declaration says. The front end's merged type of two declarations that differ in a parameter's top-level qualifiers carries none, so the encoder reads each declaration's own written type: restrict_first (restrict and a contract first, a plain redeclaration second), restrict_second (plain and the contract first, restrict second), restrict_const (restrict first, `restrict const` second: the declarations differ only in const), each called once from a function of its own (the row is at the first call, as for a bodiless declaration with one), and put_struct (a pointer to a structure: the signature is read early, so the row is at the declaration whether or not it is called; never called here). The audit's probes r01, r02, r10 and r10d (build/m4.5e1e2b/audit-R/): all four verified at the Commit 1 build
// Pre-registered from record 0064 item 19 (question 341), before any run: rows at each first call, and put_struct's at its first declaration (the line the record leaves open for two declarations: the first declaration, where the single-declaration form of this pin, restrict_reject_declaration.c's put_struct, lands; a question 160 candidate for the line, never for the outcome)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
};

void restrict_first(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5);
void restrict_first(int *p);

void restrict_second(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5);
void restrict_second(int *restrict p);

void restrict_const(int *restrict p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5);
void restrict_const(int *restrict const p);

void put_struct(struct S *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));
void put_struct(struct S *p);

int call_first(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  restrict_first(&x);  // <- here
  return x;
}

int call_second(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  restrict_second(&x);  // <- here
  return x;
}

int call_const(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  restrict_const(&x);  // <- here
  return x;
}
