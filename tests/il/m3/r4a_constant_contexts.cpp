// expect: frontend-error expr_not_constant @ L23
// expect: frontend-error expr_not_constant @ L25
// expect: frontend-error expr_not_constant @ L26
// expect: frontend-error expr_not_constant @ L27
// expect: frontend-error expr_not_constant @ L28
// expect: frontend-error expr_not_constant @ L29
// expect: frontend-error expr_not_constant @ L31
// expect: frontend-error expr_not_constant @ L32
// expect: frontend-error expr_not_constant @ L36
// expect: frontend-error expr_not_constant @ L37
// expect: frontend-error expr_not_constant @ L38
// expect: frontend-error expr_not_constant @ L44
// expect: frontend-error expr_not_constant @ L45
// expect: frontend-error expr_not_constant @ L46
// expect: frontend-error expr_not_constant @ L47
// expect: frontend-error expr_not_constant @ L48
// expect: frontend-error expr_not_constant @ L49
// expect: frontend-error expr_not_constant @ L52
// expect: frontend-error no_matching_function @ L52
// expect: frontend-error consteval_call_nonconstant @ L53
// expect: frontend-warning set_but_not_used @ L50
// expect: frontend-warning set_but_not_used @ L51
// mode: c++20
// Since cerastes registers RAF_NO_CONSTANT_EVALUATION for predicate and pure (fork commit P, pinned 2026-10-20),
// the front end refuses to evaluate p and q at compile time.  18 of the 24 contexts are EDG's expr_not_constant at
// their own line, the five that leave no trace in the IL included (L25, L26, L38, L44, L52); L52 also gets
// no_matching_function, and the consteval call at L53 gets consteval_call_nonconstant (its note points at L34).
// EDG's errors stop the back end, so the six contexts EDG accepts are not diagnosed in this file: L33 (constexpr
// body), L34 (consteval body), L40 (decltype operand), L43 (local array bound), L50 (noexcept operator) and L51
// (sizeof operand).  Each is rejected at its own line by a single-context test with the back end's
// cerastes-spec_call_in_code: tests/annotations/negative/constant_context_c12, c13, c17, c1b, c8, c16 (owner,
// 2026-10-20).  The line numbers below are those of the probe as first written; the `#line` directive keeps them.
// R4a probe: which traces does each constant context leave when it calls a constexpr
// predicate/pure function?  q is a constexpr predicate, p a constexpr pure function; each
// context below uses one of them in a constant expression.
#line 18
#include <cerastes.h>

CERASTES_PREDICATE constexpr bool q(int x) { return x >= 0; }
CERASTES_PURE constexpr int p(int x) { return x + 1; }

int g_array[p(3)];                                   // C1 array bound (global)
template <int N> struct S { int v = N; };
S<p(3)> g_tmpl_arg;                                  // C2 template argument
static_assert(q(3), "q");                            // C3 static_assert (namespace scope)
enum E1 { E1_A = p(1) };                             // C5 enumerator initializer
constexpr int g_cx = p(2);                           // C6 constexpr variable
template <int N = p(5)> struct D { int v = N; };     // C11 default template argument
D<> g_default_arg;
struct B { int bits : p(3); };                       // C10 bit-field width
alignas(p(7)) int g_aligned;                         // C9 alignas (p(7) = 8)
constexpr int cx_body(int y) { return p(y) + 1; }    // C12 constexpr function body
consteval int ce_body(int y) { return p(y); }        // C13 consteval function body

void noex_spec() noexcept(q(1));                     // C14 noexcept specifier
struct Ex { explicit(q(1)) Ex(int); };               // C15 explicit(bool)
struct CS { static_assert(q(1), "q"); };             // C3c static_assert in a class
template <int N> int ft() { return N; }
decltype(p(1)) g_decl;                               // C17 decltype operand (unevaluated)

int uses(int x) {
  int local_array[p(2)];                             // C1b array bound (local)
  S<p(1)> local_tmpl;                                // C2b template argument (local)
  static_assert(q(1), "q");                          // C3b static_assert in a body
  switch (x) { case p(1): break; default: break; }   // C4 case label
  enum E2 { E2_A = p(1) };                           // C5b enumerator in a body
  constexpr int local_cx = p(4);                     // C6b constexpr variable in a body
  if constexpr (q(3)) { x += 1; }                    // C7 if constexpr
  bool n = noexcept(q(3));                           // C8 noexcept operator
  int sz = (int)sizeof(p(1));                        // C16 sizeof operand (unevaluated)
  int ftv = ft<p(1)>();                              // C19 explicit template argument of a function
  int c = ce_body(2);                                // C13b consteval call
  int d = cx_body(1);                                // C12b constexpr call, constant-evaluated
  int e = cx_body(x);                                // C12c constexpr call, runtime
  (void)local_array; (void)local_tmpl; (void)n; (void)local_cx; (void)sz; (void)ftv;
  return x + c + d + e + (int)E2_A + (int)E1_A + g_cx;
}
