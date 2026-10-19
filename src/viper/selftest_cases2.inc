// (included by selftest.cpp) Calls, loops, returns, contract well-formedness, compound assignment, the self-test
// hooks, and the typing-rule rejections.
namespace {

const char* kIntMax = "2147483647";

void define_calls() {
  // A trusted function `g(x: int) -> int` with `requires x > 0`.
  auto g_pos = [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.req(g, k.gt(k.F(g, 0), k.num(k.i32, "0")));
    k.trusted(g);
    return g;
  };
  lc("call_precondition_ok", "a call that satisfies the callee's requires", [g_pos](Kit& k) {
    Function* g = g_pos(k);
    Function* f = k.fn("f", k.vd);
    Var* y = k.local(f, "y", k.i32);
    f->body = k.blk({k.decl(y, k.call(g, {k.num(k.i32, "1")}))});
    k.done(f);
  });
  lc("call_precondition_fails", "a call with 0 where the callee requires x > 0", [g_pos](Kit& k) {
    Function* g = g_pos(k);
    Function* f = k.fn("f", k.vd);
    Var* y = k.local(f, "y", k.i32);
    const Expr* c = k.call(g, {k.num(k.i32, "0")});
    f->body = k.blk({k.decl(y, c)});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  lc("call_postcondition_used", "the callee's ensures hold after the call", [](Kit& k) {
    Function* h = k.fn("h", k.i32, {{"x", k.i32}});
    k.req(h, k.range(k.F(h, 0), k.i32, "0", "100"));
    k.ens(h, k.eq(k.R(h), k.add(k.F(h, 0), k.num(k.i32, "1"))));
    k.trusted(h);
    Function* f = k.fn("f", k.vd);
    Var* y = k.local(f, "y", k.i32);
    f->body = k.blk({k.decl(y, k.call(h, {k.num(k.i32, "5")})), k.chk(k.eq_c(k.v(y), k.i32, "6"))});
    k.done(f);
  });
  lc("call_postcondition_wrong_use", "check(y == 7) after y = h(5) where h(x) == x + 1", [](Kit& k) {
    Function* h = k.fn("h", k.i32, {{"x", k.i32}});
    k.req(h, k.range(k.F(h, 0), k.i32, "0", "100"));
    k.ens(h, k.eq(k.R(h), k.add(k.F(h, 0), k.num(k.i32, "1"))));
    k.trusted(h);
    Function* f = k.fn("f", k.vd);
    Var* y = k.local(f, "y", k.i32);
    const Stmt* c = k.chk(k.eq_c(k.v(y), k.i32, "7"));
    f->body = k.blk({k.decl(y, k.call(h, {k.num(k.i32, "5")})), c});
    k.done(f);
    k.fail("check", c->loc);
  });
  lc("call_no_contract_gives_range", "a callee without a contract still returns a value of its type", [](Kit& k) {
    Function* h = k.fn("h", k.i32, {{"x", k.i32}});
    k.trusted(h);
    Function* f = k.fn("f", k.vd);
    Var* y = k.local(f, "y", k.i32);
    f->body = k.blk({k.decl(y, k.call(h, {k.num(k.i32, "5")})), k.chk(k.ge_c(k.v(y), k.i32, "-2147483648")), k.chk(k.le_c(k.v(y), k.i32, kIntMax))});
    k.done(f);
  });
  lc("call_nested_ok", "f(g(a)) with a > 0: both preconditions hold", [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.ens(g, k.eq(k.R(g), k.F(g, 0)));
    k.trusted(g);
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    k.trusted(f);
    Function* c = k.fn("caller", k.i32, {{"a", k.i32}});
    k.req(c, k.gt(k.F(c, 0), k.num(k.i32, "0")));
    c->body = k.blk({k.ret(k.call(f, {k.call(g, {k.pv(c, 0)})}))});
    k.done(c);
  });
  lc("call_nested_fails", "f(g(a)) with a unconstrained: f's precondition may not hold", [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.ens(g, k.eq(k.R(g), k.F(g, 0)));
    k.trusted(g);
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    k.trusted(f);
    Function* c = k.fn("caller", k.i32, {{"a", k.i32}});
    const Expr* outer = k.call(f, {k.call(g, {k.pv(c, 0)})});
    c->body = k.blk({k.ret(outer)});
    k.done(c);
    k.fail("precondition", outer->loc);
  });
  // A callee that never returns (ensures false): partial correctness makes everything after it vacuous.
  auto never = [](Kit& k) {
    Function* n = k.fn("never", k.i32, {{"x", k.i32}});
    k.ens(n, k.no());
    k.trusted(n);
    return n;
  };
  lc("call_placement_independent_obligation_comes_first", "never(a) + 100 / b: the division check does not depend on the call, so it is not hidden by it",
     [never](Kit& k) {
       Function* n = never(k);
       Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
       const Expr* d = k.div(k.num(k.i32, "100"), k.pv(f, 1));
       f->body = k.blk({k.es(k.add(k.call(n, {k.pv(f, 0)}), d))});
       k.done(f);
       k.fail("division-by-zero", d->loc);
     });
  lc("call_placement_dependent_obligation_follows", "never(a) + 1: the add depends on the call, so after a call that never returns it is unreachable",
     [never](Kit& k) {
       Function* n = never(k);
       Function* f = k.fn("f", k.vd, {{"a", k.i32}});
       f->body = k.blk({k.es(k.add(k.call(n, {k.pv(f, 0)}), k.num(k.i32, "1")))});
       k.done(f);
     });
  lc("call_placement_guard_is_a_dependency", "never(a) > 0 && 100 / b > 0: the division is guarded by the call's result, so it comes after the call",
     [never](Kit& k) {
       Function* n = never(k);
       Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
       f->body = k.blk({k.es(k.land(k.gt(k.call(n, {k.pv(f, 0)}), k.num(k.i32, "0")),
                                    k.gt(k.div(k.num(k.i32, "100"), k.pv(f, 1)), k.num(k.i32, "0"))))});
       k.done(f);
     });
  lc("call_argument_obligation_before_call", "g(a + b): the overflow of the argument is reported even though the callee never returns",
     [never](Kit& k) {
       Function* n = never(k);
       Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
       const Expr* s = k.add(k.pv(f, 0), k.pv(f, 1));
       f->body = k.blk({k.es(k.call(n, {s}))});
       k.done(f);
       k.fail("overflow", s->loc);
     });
  lc("call_void_statement", "a void callee with a requires", [](Kit& k) {
    Function* v = k.fn("v", k.vd, {{"x", k.i32}});
    k.req(v, k.gt(k.F(v, 0), k.num(k.i32, "0")));
    k.trusted(v);
    Function* f = k.fn("f", k.vd);
    const Expr* c = k.call(v, {k.num(k.i32, "0")});
    f->body = k.blk({k.es(k.call(v, {k.num(k.i32, "3")})), k.es(c)});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  lc("call_argument_conversion_checked", "g(c) where c is an int and the parameter is a signed char", [](Kit& k) {
    Function* g = k.fn("g", k.vd, {{"x", k.i8}});
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"c", k.i32}});
    const Expr* conv = k.cv(k.i8, k.pv(f, 0));
    f->body = k.blk({k.es(k.call(g, {conv}))});
    k.done(f);
    k.fail("conversion", conv->loc);
  });
  lc("call_default_argument_reported_at_call", "an argument from a default argument: its obligations are reported at the call", [](Kit& k) {
    Function* d = k.fn("d", k.vd, {{"x", k.i32}});
    k.trusted(d);
    Function* f = k.fn("f", k.vd);
    const Expr* inner = k.add(k.num(k.i32, kIntMax), k.num(k.i32, "1"));  // at the declaration of the default argument
    const Expr* at_call = k.b.report_at(inner, k.at());
    f->body = k.blk({k.es(k.call(d, {at_call}))});
    k.done(f);
    k.fail_with_note("overflow", at_call->loc, inner->loc.line);
  });
  lc("call_recursion", "a recursive function with a contract and a measure: total correctness", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq_c(k.R(f), k.i32, "0"));
    k.dec(f, k.F(f, 0));
    Var* r = k.local(f, "r", k.i32);
    f->body = k.blk({k.iff(k.eq_c(k.pv(f, 0), k.i32, "0"), k.blk({k.ret(k.num(k.i32, "0"))})),
                     k.decl(r, k.call(f, {k.sub(k.pv(f, 0), k.num(k.i32, "1"))})), k.ret(k.v(r))});
    k.done(f);
  });
  lc("call_recursion_bad_argument", "f(n - 1) without the base case: n - 1 may be negative", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.dec(f, k.F(f, 0));
    Var* r = k.local(f, "r", k.i32);
    const Expr* c = k.call(f, {k.sub(k.pv(f, 0), k.num(k.i32, "1"))});
    f->body = k.blk({k.decl(r, c), k.ret(k.v(r))});
    k.done(f);
    k.fail("precondition", c->loc);
  });
}

void define_loops() {
  // count(n): i from 0 to n.
  auto count = [](Kit& k, const char* inv_text, bool want_entry, bool want_preserved) {
    Function* f = k.fn("count", k.i32, {{"n", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    if (std::string(inv_text) == "good") k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* d = k.decl(i, k.num(k.i32, "0"));
    const Expr* inv = nullptr;
    std::string t = inv_text;
    if (t == "good") inv = k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 0)));
    else if (t == "eq1") inv = k.eq_c(k.v(i), k.i32, "1");
    else if (t == "le5") inv = k.le_c(k.v(i), k.i32, "5");
    Clause c = k.cl(inv);
    const Stmt* w = k.whl(k.lt(k.v(i), k.pv(f, 0)), {c}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))}, k.blk({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}));
    f->body = k.blk({d, w, k.ret(k.v(i))});
    k.done(f);
    if (want_entry) k.fail("invariant-entry", c.loc);
    if (want_preserved) k.fail("invariant-preserved", c.loc);
  };
  lc("loop_count_ok", "while (i < n) i = i + 1 with invariant 0 <= i <= n: returns n", [count](Kit& k) { count(k, "good", false, false); });
  lc("loop_invariant_entry_fails", "invariant i == 1 with i = 0 initially", [count](Kit& k) { count(k, "eq1", true, false); });
  lc("loop_invariant_not_preserved", "invariant i <= 5 holds on entry but the loop runs to n", [count](Kit& k) { count(k, "le5", false, true); });
  lc("loop_condition_obligation_ok", "while (100 / n > 1): n != 0 is an invariant clause, proved from 1 <= n <= 1000", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "1000"));
    const Var* n = f->param_var(0);
    Clause c = k.cl(k.range(k.v(n), k.i32, "1", "1000"));
    f->body = k.blk({k.whl(k.gt(k.div(k.num(k.i32, "100"), k.v(n)), k.num(k.i32, "1")), {c}, {k.cl(k.sub(k.num(k.i32, "100"), k.v(n)))},
                           k.blk({k.asg(n, k.add(k.v(n), k.num(k.i32, "1")))}))});
    k.done(f);
  });
  lc("loop_condition_obligation_fails_on_entry", "while (100 / n > 1) with n >= 0 only: n may be 0 on entry", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    const Var* n = f->param_var(0);
    Clause c = k.cl(k.range(k.v(n), k.i32, "0", "1000"));
    const Expr* d = k.div(k.num(k.i32, "100"), k.v(n));
    f->body = k.blk({k.whl(k.gt(d, k.num(k.i32, "1")), {c}, {k.cl(k.sub(k.num(k.i32, "100"), k.v(n)))}, k.blk({k.asg(n, k.add(k.v(n), k.num(k.i32, "1")))}))});
    k.done(f);
    k.fail("division-by-zero", d->loc, 1, "invariant.not.established");
  });
  lc("loop_auto_range_invariant_is_needed", "c = (signed char)(c + 1) while c < 10: the lower bound of c comes from the automatic invariant", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* c = k.local(f, "c", k.i8);
    f->body = k.blk({k.decl(c, k.num(k.i8, "0")),
                     k.whl(k.lt(k.v(c), k.num(k.i8, "10")), {}, {k.cl(k.sub(k.num(k.i32, "10"), k.cv(k.i32, k.v(c))))},
                           k.blk({k.asg(c, k.cv(k.i8, k.add(k.cv(k.i32, k.v(c)), k.num(k.i32, "1"))))}))});
    k.done(f);
  });
  lc("loop_auto_range_invariant_hook", "the same loop with the automatic invariant weakened (a self-test hook): Silicon rejects it", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* c = k.local(f, "c", k.i8);
    const Stmt* w = k.whl(k.lt(k.v(c), k.num(k.i8, "10")), {}, {k.cl(k.sub(k.num(k.i32, "10"), k.cv(k.i32, k.v(c))))},
                          k.blk({k.asg(c, k.cv(k.i8, k.add(k.cv(k.i32, k.v(c)), k.num(k.i32, "1"))))}));
    f->body = k.blk({k.decl(c, k.num(k.i8, "0")), w});
    k.done(f);
    k.lower_options.weaken_auto_invariant = true;
    k.fail("internal", w->loc, 1, "invariant.not.established");
  });
  lc("loop_without_assigned_vars_has_no_auto_invariant", "a loop that assigns nothing", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));  // the body is then unreachable, so the loop terminates
    f->body = k.blk({k.whl(k.lt(k.pv(f, 0), k.num(k.i32, "0")), {}, {k.cl(k.cv(k.i64, k.pv(f, 0)))}, k.blk({}))});
    k.done(f);
  });
  lc("loop_nested_outer_variable", "an outer variable assigned only in an inner loop keeps its range", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "10"));
    Var* s = k.local(f, "s", k.i8);
    Var* i = k.local(f, "i", k.i32);
    Var* j = k.local(f, "j", k.i32);
    const Stmt* inner = k.whl(k.lt(k.v(j), k.pv(f, 0)),
                              {k.cl(k.range(k.v(j), k.i32, "0", "10"))}, {k.cl(k.gap(k.pv(f, 0), k.v(j)))},
                              k.blk({k.asg(s, k.cv(k.i8, k.add(k.cv(k.i32, k.v(s)), k.num(k.i32, "0")))), k.asg(j, k.add(k.v(j), k.num(k.i32, "1")))}));
    const Stmt* outer = k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "10"))}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                              k.blk({k.decl(j, k.num(k.i32, "0")), inner, k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}));
    f->body = k.blk({k.decl(s, k.num(k.i8, "0")), k.decl(i, k.num(k.i32, "0")), outer});
    k.done(f);
  });
  lc("loop_invariant_obligation_guarded", "invariants d > 0 and 100 / d >= 0: the division is guarded by the first invariant", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"d", k.i32}});
    k.req(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    const Var* d = f->param_var(0);
    f->body = k.blk({k.whl(k.lt(k.v(d), k.num(k.i32, "0")),
                           {k.cl(k.gt(k.v(d), k.num(k.i32, "0"))), k.cl(k.ge(k.div(k.num(k.i32, "100"), k.v(d)), k.num(k.i32, "0")))},
                           {k.cl(k.cv(k.i64, k.v(d)))}, k.blk({}))});  // the invariant d > 0 makes the empty body unreachable
    k.done(f);
  });
  lc("loop_invariant_obligation_unguarded", "invariants 100 / d >= 0 and d > 0: the division comes first, unguarded", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"d", k.i32}});
    const Var* d = f->param_var(0);
    const Expr* div = k.div(k.num(k.i32, "100"), k.v(d));
    f->body = k.blk({k.whl(k.lt(k.v(d), k.num(k.i32, "0")), {k.cl(k.lt(div, k.num(k.i32, "1000"))), k.cl(k.gt(k.v(d), k.num(k.i32, "0")))},
                           {k.cl(k.cv(k.i64, k.v(d)))}, k.blk({}))});
    k.done(f);
    k.fail("division-by-zero", div->loc, 1, "invariant.not.established");
    k.fail("invariant-entry", f->body->stmts[0]->invariants[1].loc);
  });
  lc("loop_obligations_in_body", "an ordinary obligation inside a loop body", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "1"));  // s grows, so the loop terminates (the overflow of s + n is the point)
    Var* s = k.local(f, "s", k.i32);
    const Expr* a = k.add(k.v(s), k.pv(f, 0));
    f->body = k.blk({k.decl(s, k.num(k.i32, "0")),
                     k.whl(k.lt(k.v(s), k.num(k.i32, "10")), {}, {k.cl(k.sub(k.num(k.i64, "10"), k.cv(k.i64, k.v(s))))}, k.blk({k.asg(s, a)}))});
    k.done(f);
    k.fail("overflow", a->loc);
  });
  lc("loop_return_inside", "a return inside a loop body jumps out of it and asserts the postcondition there", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    k.ens(f, k.range(k.R(f), k.i32, "0", "100"));
    Var* i = k.local(f, "i", k.i32);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100"))}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.iff(k.gt(k.v(i), k.num(k.i32, "5")), k.blk({k.ret(k.v(i))})), k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))})),
                     k.ret(k.num(k.i32, "0"))});
    k.done(f);
  });
  lc("loop_return_inside_bad_postcondition", "the return inside the loop violates ensures result < 3", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    k.ens(f, k.lt(k.R(f), k.num(k.i32, "3")));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* r = k.ret(k.v(i));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100"))}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.iff(k.gt(k.v(i), k.num(k.i32, "5")), k.blk({r})), k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))})),
                     k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail("postcondition", r->loc);
  });
  lc("loop_for_style_declaration", "for (int i = 0; i < n; ++i) as `init; while (cond) { body; incr }`", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    k.ens(f, k.eq(k.R(f), k.mul(k.F(f, 0), k.num(k.i32, "2"))));
    Var* s = k.local(f, "s", k.i32);
    Var* i = k.local(f, "i", k.i32);
    f->body = k.blk({k.decl(s, k.num(k.i32, "0")), k.decl(i, k.num(k.i32, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 0)),
                           {k.cl(k.range(k.v(i), k.i32, "0", "1000")), k.cl(k.le(k.v(i), k.pv(f, 0))), k.cl(k.eq(k.v(s), k.mul(k.v(i), k.num(k.i32, "2"))))},
                           {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.asg(s, k.add(k.v(s), k.num(k.i32, "2"))), k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))})),
                     k.ret(k.v(s))});
    k.done(f);
  });
}

void define_returns_and_contracts() {
  lc("missing_return_fails", "if (a > 0) return 1; and then the end of the function", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    f->body = k.blk({k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({k.ret(k.num(k.i32, "1"))}))});
    k.done(f);
    k.fail("missing-return", f->end_loc);
  });
  lc("missing_return_ok", "if/else, both returning", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    f->body = k.blk({k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({k.ret(k.num(k.i32, "1"))}), k.blk({k.ret(k.num(k.i32, "2"))}))});
    k.done(f);
  });
  lc("missing_return_after_loop_fails", "a loop and then no return", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));  // the loop body is then unreachable (the loop terminates)
    f->body = k.blk({k.whl(k.lt(k.pv(f, 0), k.num(k.i32, "0")), {}, {k.cl(k.cv(k.i64, k.pv(f, 0)))}, k.blk({}))});
    k.done(f);
    k.fail("missing-return", f->end_loc);
  });
  lc("void_postcondition_at_end_fails", "a void function whose ensures a > 0 is not guaranteed", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.ens(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    f->body = k.blk({});
    k.done(f);
    k.fail("postcondition", f->end_loc);
  });
  lc("void_postcondition_ok", "the same with requires a > 0, and a `return;` in the middle", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.req(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    k.ens(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    f->body = k.blk({k.iff(k.lt(k.pv(f, 0), k.num(k.i32, "5")), k.blk({k.ret(nullptr)}))});
    k.done(f);
  });
  lc("postcondition_fails_at_return", "return a with ensures result > a", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.ens(f, k.gt(k.R(f), k.F(f, 0)));
    const Stmt* r = k.ret(k.pv(f, 0));
    f->body = k.blk({r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });
  lc("postcondition_each_return", "two returns; the second violates the postcondition", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.ge(k.R(f), k.F(f, 0)));
    const Stmt* r1 = k.ret(k.add(k.pv(f, 0), k.num(k.i32, "1")));
    const Stmt* r2 = k.ret(k.sub(k.pv(f, 0), k.num(k.i32, "1")));
    f->body = k.blk({k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "50")), k.blk({r1})), r2});
    k.done(f);
    k.fail("postcondition", r2->loc);
  });
  lc("postcondition_reads_entry_values", "a parameter assigned in the body: the contract still means its value on entry", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), k.num(k.i32, "1"))));
    k.ens(f, k.eq(k.R(f), k.add(k.old(k.F(f, 0)), k.num(k.i32, "1"))));
    const Var* a = f->param_var(0);
    f->body = k.blk({k.asg(a, k.add(k.v(a), k.num(k.i32, "1"))), k.ret(k.v(a))});
    k.done(f);
  });
  lc("postcondition_wrong_entry_value", "ensures result == a (the entry value) after a = a + 1", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    const Var* a = f->param_var(0);
    const Stmt* r = k.ret(k.v(a));
    f->body = k.blk({k.asg(a, k.add(k.v(a), k.num(k.i32, "1"))), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });
  lc("check_reads_current_values", "check(a == 6) after a = a + 1 with a == 5 on entry", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.req(f, k.eq_c(k.F(f, 0), k.i32, "5"));
    const Var* a = f->param_var(0);
    f->body = k.blk({k.asg(a, k.add(k.v(a), k.num(k.i32, "1"))), k.chk(k.eq_c(k.v(a), k.i32, "6"))});
    k.done(f);
  });
  lc("check_obligation_is_reported", "check(100 / a > 0) with a unconstrained", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    const Expr* d = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    f->body = k.blk({k.chk(k.lt(d, k.num(k.i32, "1000")))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("bool_result_postcondition", "a bool function and its result in ensures", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    k.ens(f, k.eq(k.R(f), k.gt(k.F(f, 0), k.num(k.i32, "0"))));
    f->body = k.blk({k.ret(k.gt(k.pv(f, 0), k.num(k.i32, "0")))});
    k.done(f);
  });
  lc("hook_drop_postcondition_assert", "the postcondition assert is dropped (a self-test hook): Silicon's own check of the method ensures fires", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.ens(f, k.gt(k.R(f), k.F(f, 0)));
    f->body = k.blk({k.ret(k.pv(f, 0))});
    k.done(f);
    k.lower_options.drop_assert = 0;
    k.fail("internal", f->ensures_[0].loc, 1, "postcondition.violated");
  });
  lc("hook_drop_precondition_assert", "the call's precondition assert is dropped: Silicon's own check of the callee's requires fires", [](Kit& k) {
    Function* g = k.fn("g", k.vd, {{"x", k.i32}});
    k.req(g, k.gt(k.F(g, 0), k.num(k.i32, "0")));
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    const Expr* c = k.call(g, {k.num(k.i32, "0")});
    f->body = k.blk({k.es(c)});
    k.done(f);
    k.lower_options.drop_assert = 0;
    k.fail("internal", c->loc, 1, "call.precondition");
  });
}

void define_wellformedness() {
  lc("wf_requires_overflow", "requires a + b > 0 with a, b unconstrained: the contract itself is undefined", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
    const Expr* s = k.add(k.F(f, 0), k.F(f, 1));
    k.req(f, k.gt(s, k.num(k.i32, "0")));
    f->body = k.blk({});
    k.done(f);
    k.fail("overflow", s->loc);
  });
  lc("wf_requires_ok", "requires a + b > 0 with both bounded", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "1000"));
    k.req(f, k.gt(k.add(k.F(f, 0), k.F(f, 1)), k.num(k.i32, "0")));
    f->body = k.blk({});
    k.done(f);
  });
  lc("wf_requires_order_matters", "requires a + b > 0 before the clauses that bound a and b", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
    const Expr* s = k.add(k.F(f, 0), k.F(f, 1));
    k.req(f, k.gt(s, k.num(k.i32, "0")));
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "1000"));
    f->body = k.blk({});
    k.done(f);
    k.fail("overflow", s->loc);
  });
  lc("wf_ensures_guarded_by_requires", "ensures result == 100 / a with requires a != 0 and a > 0", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    k.ens(f, k.eq(k.R(f), k.div(k.num(k.i32, "100"), k.F(f, 0))));
    f->body = k.blk({k.ret(k.div(k.num(k.i32, "100"), k.pv(f, 0)))});
    k.done(f);
  });
  lc("wf_ensures_unguarded", "ensures result == 100 / a with a unconstrained: the contract and the body both divide", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    const Expr* in_ens = k.div(k.num(k.i32, "100"), k.F(f, 0));
    k.ens(f, k.eq(k.R(f), in_ens));
    const Expr* in_body = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(in_body)});
    k.done(f);
    k.fail("division-by-zero", in_ens->loc);
    k.fail("division-by-zero", in_body->loc);
  });
  lc("wf_trusted_contract_is_checked_too", "a trusted contract whose ensures overflows", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    const Expr* s = k.add(k.F(f, 0), k.F(f, 1));
    k.ens(f, k.eq(k.R(f), s));
    k.trusted(f);
    k.fail("overflow", s->loc);
  });
  lc("wf_ensures_may_assume_earlier_ensures", "ensures a < 100, then ensures result + a > 0 can use it", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.range(k.R(f), k.i32, "0", "100"));
    k.ens(f, k.ge(k.add(k.R(f), k.F(f, 0)), k.num(k.i32, "0")));
    f->body = k.blk({k.ret(k.num(k.i32, "0"))});
    k.done(f);
  });
}

void define_compound_assignment() {
  // A function over one parameter `c` of type `t`, running `body(c, loc)` and expecting the failures it declares.
  auto ca = [](Kit& k, Type t, const std::string& lo, const std::string& hi, bool bounded,
               const std::function<const Stmt*(Kit&, const Var*, const SrcLoc&)>& make) {
    Function* f = k.fn("f", k.vd, {{"c", t}});
    if (bounded) k.req(f, k.range(k.F(f, 0), t, lo, hi));
    const SrcLoc l = k.at();
    f->body = k.blk({make(k, f->param_var(0), l)});
    k.done(f);
    return l;
  };
  lc("ca_i8_add_unbounded", "c += 1 on signed char: computed in int, converted back: may not fit", [ca](Kit& k) {
    SrcLoc l = ca(k, k.i8, "", "", false, [](Kit& k, const Var* c, const SrcLoc& l) { return k.b.compound_assign(c, BinaryOp::Add, k.num(k.i32, "1"), l); });
    k.fail("conversion", l);
  });
  lc("ca_i8_add_bounded", "c += 1 on signed char with c <= 100", [ca](Kit& k) {
    ca(k, k.i8, "-128", "100", true, [](Kit& k, const Var* c, const SrcLoc& l) { return k.b.compound_assign(c, BinaryOp::Add, k.num(k.i32, "1"), l); });
  });
  lc("ca_u16_mul_overflows_int", "us *= us on unsigned short is computed in int and can overflow", [ca](Kit& k) {
    SrcLoc l = ca(k, k.u16, "", "", false, [](Kit& k, const Var* c, const SrcLoc& l) {
      return k.b.compound_assign(c, BinaryOp::Mul, k.cv(k.i32, k.v(c)), l);
    });
    k.fail("overflow", l);
  });
  lc("ca_u16_mul_bounded", "us *= us with us <= 40000: fits int, and converting back to unsigned short wraps", [ca](Kit& k) {
    ca(k, k.u16, "0", "40000", true, [](Kit& k, const Var* c, const SrcLoc& l) {
      return k.b.compound_assign(c, BinaryOp::Mul, k.cv(k.i32, k.v(c)), l);
    });
  });
  lc("ca_i8_shl_unbounded", "c <<= 1 on signed char: negative operand, and the result may not fit signed char", [ca](Kit& k) {
    SrcLoc l = ca(k, k.i8, "", "", false, [](Kit& k, const Var* c, const SrcLoc& l) { return k.b.compound_assign(c, BinaryOp::Shl, k.num(k.i32, "1"), l); });
    k.fail("shift", l, 1, "", "left operand");
    k.fail("conversion", l);
  });
  lc("ca_i8_shl_bounded", "c <<= 1 with 0 <= c <= 63", [ca](Kit& k) {
    ca(k, k.i8, "0", "63", true, [](Kit& k, const Var* c, const SrcLoc& l) { return k.b.compound_assign(c, BinaryOp::Shl, k.num(k.i32, "1"), l); });
  });
  lc("ca_i8_shl_does_not_fit_back", "c <<= 1 with 0 <= c <= 100: 200 does not fit signed char", [ca](Kit& k) {
    SrcLoc l = ca(k, k.i8, "0", "100", true, [](Kit& k, const Var* c, const SrcLoc& l) { return k.b.compound_assign(c, BinaryOp::Shl, k.num(k.i32, "1"), l); });
    k.fail("conversion", l);
  });
  lc("ca_shift_count_keeps_its_type", "i <<= n with n a long: the count is checked as it is, the left operand promotes alone", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"i", k.i32}, {"n", k.i64}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    k.req(f, k.range(k.F(f, 1), k.i64, "0", "10"));
    f->body = k.blk({k.b.compound_assign(f->param_var(0), BinaryOp::Shl, k.pv(f, 1), k.at())});
    k.done(f);
  });
  lc("ca_i32_add_u32_converts_back", "i += u with u unsigned: computed in unsigned int, converted back to int", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"i", k.i32}, {"u", k.u32}});
    const SrcLoc l = k.at();
    f->body = k.blk({k.b.compound_assign(f->param_var(0), BinaryOp::Add, k.pv(f, 1), l)});
    k.done(f);
    k.fail("conversion", l);
  });
  lc("ca_i32_add_u32_bounded", "i += u with 0 <= i <= 1000 and u <= 1000", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"i", k.i32}, {"u", k.u32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    k.req(f, k.le_c(k.F(f, 1), k.u32, "1000"));
    f->body = k.blk({k.b.compound_assign(f->param_var(0), BinaryOp::Add, k.pv(f, 1), k.at())});
    k.done(f);
  });
  lc("ca_i32_add_u32_negative_wraps", "int i = -1; i += 1u: (unsigned)-1 + 1 == 0, converted back to 0", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* i = k.local(f, "i", k.i32);
    f->body = k.blk({k.decl(i, k.num(k.i32, "-1")), k.b.compound_assign(i, BinaryOp::Add, k.num(k.u32, "1"), k.at()), k.chk(k.eq_c(k.v(i), k.i32, "0"))});
    k.done(f);
  });
  lc("ca_u8_mul_wraps", "unsigned char uc = 16; uc *= uc;  uc == 0 (256 mod 256)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* uc = k.local(f, "uc", k.u8);
    f->body = k.blk({k.decl(uc, k.num(k.u8, "16")), k.b.compound_assign(uc, BinaryOp::Mul, k.cv(k.i32, k.v(uc)), k.at()), k.chk(k.eq_c(k.v(uc), k.u8, "0"))});
    k.done(f);
  });
  lc("ca_u8_mul_wrong", "the same, then check(uc == 1)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* uc = k.local(f, "uc", k.u8);
    const Stmt* c = k.chk(k.eq_c(k.v(uc), k.u8, "1"));
    f->body = k.blk({k.decl(uc, k.num(k.u8, "16")), k.b.compound_assign(uc, BinaryOp::Mul, k.cv(k.i32, k.v(uc)), k.at()), c});
    k.done(f);
    k.fail("check", c->loc);
  });
  lc("ca_div_by_zero_is_reported", "x /= 0", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"x", k.i32}});
    const SrcLoc l = k.at();
    f->body = k.blk({k.b.compound_assign(f->param_var(0), BinaryOp::Div, k.num(k.i32, "0"), l)});
    k.done(f);
    k.fail("division-by-zero", l);
  });
  lc("incdec_i8_at_max", "signed char c = 127; ++c: the result does not fit", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* c = k.local(f, "c", k.i8);
    const SrcLoc l = k.at();
    f->body = k.blk({k.decl(c, k.num(k.i8, "127")), k.b.inc_dec(c, true, l)});
    k.done(f);
    k.fail("conversion", l);
  });
  lc("incdec_i8_below_max", "signed char c = 126; ++c; c == 127", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* c = k.local(f, "c", k.i8);
    f->body = k.blk({k.decl(c, k.num(k.i8, "126")), k.b.inc_dec(c, true, k.at()), k.chk(k.eq_c(k.v(c), k.i8, "127"))});
    k.done(f);
  });
  lc("incdec_u32_wraps_down", "unsigned x = 0; --x; x == UINT_MAX", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.local(f, "x", k.u32);
    f->body = k.blk({k.decl(x, k.num(k.u32, "0")), k.b.inc_dec(x, false, k.at()), k.chk(k.eq_c(k.v(x), k.u32, "4294967295"))});
    k.done(f);
  });
  lc("incdec_i32_at_max", "int i = INT_MAX; i++: overflow, at the operator", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* i = k.local(f, "i", k.i32);
    const SrcLoc l = k.at();
    f->body = k.blk({k.decl(i, k.num(k.i32, kIntMax)), k.b.inc_dec(i, true, l)});
    k.done(f);
    k.fail("overflow", l);
  });
  lc("shl_u128_top_bit", "1 << 127 on unsigned __int128 and 1 << 126 on __int128 are exact", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.eq_c(k.shl(k.num(k.u128, "1"), k.num(k.i32, "127")), k.u128, "170141183460469231731687303715884105728")),
                     k.chk(k.eq_c(k.shl(k.num(k.i128, "1"), k.num(k.i32, "126")), k.i128, "85070591730234615865843651857942052864"))});
    k.done(f);
  });
  lc("shl_i128_sign_bit", "1 << 127 on __int128 does not fit", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* e = k.shl(k.num(k.i128, "1"), k.num(k.i32, "127"));
    f->body = k.blk({k.es(e)});
    k.done(f);
    k.fail("shift", e->loc, 1, "", "may not fit");
  });
  lc("promotion_rules", "char/short promote to int; mixed signedness follows (bits, signedness)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    // c16 += c16 on unsigned short is int; i64 += u32 is long; u64 += i32 is unsigned long (long long + unsigned long: unsigned).
    Var* a = k.local(f, "a", k.i64);
    Var* b = k.local(f, "b", k.u64);
    f->body = k.blk({k.decl(a, k.num(k.i64, "-1")), k.decl(b, k.num(k.u64, "0")),
                     k.b.compound_assign(a, BinaryOp::Add, k.num(k.u32, "5"), k.at()), k.chk(k.eq_c(k.v(a), k.i64, "4")),
                     k.b.compound_assign(b, BinaryOp::Sub, k.num(k.i32, "1"), k.at()), k.chk(k.eq_c(k.v(b), k.u64, "18446744073709551615"))});
    k.done(f);
  });
}

void define_misc() {
  lc("stmt_if_else_and_blocks", "if/else with blocks and a decl in each branch", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.ens(f, k.range(k.R(f), k.i32, "0", "1"));
    Var* r = k.local(f, "r", k.i32);
    Var* s = k.local(f, "s", k.i32);
    f->body = k.blk({k.decl(r, k.num(k.i32, "0")),
                     k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({k.decl(s, k.num(k.i32, "1")), k.asg(r, k.v(s))}), k.blk({k.asg(r, k.num(k.i32, "0"))})),
                     k.ret(k.v(r))});
    k.done(f);
  });
  lc("expr_stmt_obligations_only", "a discarded expression is still evaluated: x / 0;", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"x", k.i32}});
    const Expr* d = k.div(k.pv(f, 0), k.num(k.i32, "0"));
    f->body = k.blk({k.es(d)});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("unnamed_parameter_and_odd_names", "an unnamed parameter and a name with a dollar sign", [](Kit& k) {
    Function* f = k.fn("a$b", k.i32, {{"", k.i32}, {"x$", k.i32}});
    f->body = k.blk({k.ret(k.pv(f, 0))});
    k.done(f);
  });
  lc("deterministic_int_types_shapes", "types of the same shape but different names are one type", [](Kit& k) {
    Type wchar = Type::integer(k.prog.int_type("wchar_t", 32, true));  // interned: the same type as int
    Function* f = k.fn("f", k.i32, {{"w", wchar}});
    f->body = k.blk({k.ret(k.add(k.pv(f, 0), k.num(k.i32, "1")))});
    k.done(f);
    // add(w, 1) may overflow: w is unconstrained
    k.fail("overflow", f->body->stmts[0]->expr->loc);
  });
}

void define_termination() {
  // Decision 0013: loops and recursion carry a measure, and Silicon checks it.
  lc("term_loop_measure_does_not_decrease", "while (i < n) {}: the measure n - i never changes", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "1"));
    Var* i = k.local(f, "i", k.i32);
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.whl(k.lt(k.v(i), k.pv(f, 0)), {}, {m}, k.blk({}))});
    k.done(f);
    k.fail("termination", m.loc);
  });
  lc("term_loop_measure_not_bounded", "i = 3; while (i != -5) i = i - 1 with measure i: it decreases but goes below zero", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* i = k.local(f, "i", k.i32);
    Clause m = k.cl(k.cv(k.i64, k.v(i)));
    f->body = k.blk({k.decl(i, k.num(k.i32, "3")),
                     k.whl(k.ne(k.v(i), k.num(k.i32, "-5")), {k.cl(k.range(k.v(i), k.i32, "-5", "3"))}, {m},
                           k.blk({k.asg(i, k.sub(k.v(i), k.num(k.i32, "1")))}))});
    k.done(f);
    k.fail("termination", m.loc);
  });
  lc("term_loop_negative_counter_ok", "i = -3; while (i < 0) i = i + 1 with measure 0 - i", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* i = k.local(f, "i", k.i32);
    f->body = k.blk({k.decl(i, k.num(k.i32, "-3")),
                     k.whl(k.lt(k.v(i), k.num(k.i32, "0")), {k.cl(k.range(k.v(i), k.i32, "-3", "0"))},
                           {k.cl(k.sub(k.num(k.i64, "0"), k.cv(k.i64, k.v(i))))}, k.blk({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))});
    k.done(f);
  });
  lc("term_loop_left_by_return_is_checked", "a loop whose measure does not decrease on the path that does not return", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    Var* i = k.local(f, "i", k.i32);
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100"))}, {m},
                           k.blk({k.iff(k.gt(k.v(i), k.num(k.i32, "5")), k.blk({k.ret(k.v(i))}))})),
                     k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail("termination", m.loc);
  });
  lc("term_recursion_not_decreasing", "f(n) calls f(n): the measure does not decrease", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.dec(f, k.F(f, 0));
    const Expr* c = k.call(f, {k.pv(f, 0)});
    f->body = k.blk({k.ret(c)});
    k.done(f);
    k.fail("termination", c->loc);
  });
  auto mutual = [](Kit& k, bool decreasing) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    Function* g = k.fn("g", k.i32, {{"n", k.i32}});
    for (Function* h : {f, g}) {
      k.req(h, k.range(k.F(h, 0), k.i32, "0", "100"));
      k.dec(h, k.F(h, 0));
    }
    f->body = k.blk({k.iff(k.eq_c(k.pv(f, 0), k.i32, "0"), k.blk({k.ret(k.num(k.i32, "0"))})),
                     k.ret(k.call(g, {k.sub(k.pv(f, 0), k.num(k.i32, "1"))}))});
    k.done(f);
    const Expr* back = k.call(f, {decreasing ? k.sub(k.pv(g, 0), k.num(k.i32, "1")) : k.pv(g, 0)});
    g->body = k.blk({k.iff(k.eq_c(k.pv(g, 0), k.i32, "0"), k.blk({k.ret(k.num(k.i32, "1"))})), k.ret(back)});
    k.done(g);
    if (!decreasing) k.fail("termination", back->loc);
  };
  lc("term_mutual_recursion_ok", "f(n) calls g(n - 1) and g(n) calls f(n - 1)", [mutual](Kit& k) { mutual(k, true); });
  lc("term_mutual_recursion_not_decreasing", "g(n) calls f(n): the measure does not decrease on that call", [mutual](Kit& k) { mutual(k, false); });
  lc("term_lexicographic_ok", "lex(a, b) calls lex(a, b - 1) or lex(a - 1, 100): decreasing lexicographically", [](Kit& k) {
    Function* f = k.fn("lex", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "100"));
    k.dec(f, k.F(f, 0));
    k.dec(f, k.F(f, 1));
    const Expr* c1 = k.call(f, {k.pv(f, 0), k.sub(k.pv(f, 1), k.num(k.i32, "1"))});
    const Expr* c2 = k.call(f, {k.sub(k.pv(f, 0), k.num(k.i32, "1")), k.num(k.i32, "100")});
    f->body = k.blk({k.iff(k.gt(k.pv(f, 1), k.num(k.i32, "0")), k.blk({k.ret(c1)})),
                     k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({k.ret(c2)})), k.ret(k.num(k.i32, "0"))});
    k.done(f);
  });
  lc("term_measure_obligation_in_loop", "a measure whose own arithmetic overflows: an obligation at the loop head", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "1000000"));
    Var* i = k.local(f, "i", k.i32);
    const Expr* mul = k.mul(k.sub(k.pv(f, 0), k.v(i)), k.num(k.i32, "100000"));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "1000000"))}, {k.cl(mul)},
                           k.blk({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))});
    k.done(f);
    k.fail("overflow", mul->loc, 1, "invariant.not.established");
  });
  lc("term_measure_obligation_in_function", "decreases n * 4 with n up to 10^9: the measure is checked in the well-formedness method", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000000000"));
    const Expr* mul = k.mul(k.F(f, 0), k.num(k.i32, "4"));
    k.dec(f, mul);
    f->body = k.blk({k.ret(k.pv(f, 0))});
    k.done(f);
    k.fail("overflow", mul->loc);
  });
}

void define_calls_loops_and_more() {
  define_calls();
  define_loops();
  define_termination();
  define_returns_and_contracts();
  define_wellformedness();
  define_compound_assignment();
  define_misc();
}

// ---- typing-rule rejections -----------------------------------------------------------------------------------

void define_rejections() {
  auto one = [](Kit& k, const std::function<const Expr*(Kit&, Function*)>& e, Type ret = Type()) {
    Function* f = k.fn("f", ret.is_void() ? k.vd : ret, {{"a", k.i32}, {"u", k.u32}, {"b", k.bl}});
    const Expr* x = e(k, f);
    f->body = k.blk({k.es(x)});
    k.done(f);
    return f;
  };
  rc("mixed_signedness_add", "both must be the same integer type", [one](Kit& k) { one(k, [](Kit& k, Function* f) { return k.add(k.pv(f, 0), k.pv(f, 1)); }); });
  rc("mixed_width_compare", "both must be the same integer type", [one](Kit& k) {
    one(k, [](Kit& k, Function* f) { return k.lt(k.pv(f, 0), k.cv(k.i64, k.pv(f, 0))); });
  });
  rc("bool_shift_count", "both must be integers", [one](Kit& k) { one(k, [](Kit& k, Function* f) { return k.shl(k.pv(f, 0), k.pv(f, 2)); }); });
  rc("formal_in_body", "formal parameter outside a contract", [one](Kit& k) { one(k, [](Kit& k, Function* f) { return k.F(f, 0); }); });
  rc("result_in_body", "the result outside an `ensures`", [one](Kit& k) {
    one(k, [](Kit& k, Function* f) { return k.R(f); }, k.i32);
  });
  rc("local_in_contract", "local variable in a contract", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.req(f, k.gt(k.pv(f, 0), k.num(k.i32, "0")));
    f->body = k.blk({});
    k.done(f);
  });
  rc("result_in_requires", "outside an `ensures`", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.gt(k.R(f), k.num(k.i32, "0")));
    f->body = k.blk({k.ret(k.num(k.i32, "0"))});
    k.done(f);
  });
  rc("old_of_result", "`old` of the result", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.ens(f, k.gt(k.old(k.R(f)), k.num(k.i32, "0")));
    f->body = k.blk({k.ret(k.num(k.i32, "0"))});
    k.done(f);
  });
  rc("call_in_check", "a call in a loop condition or in a `check`", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.call(g, {}))});
    k.done(f);
  });
  rc("call_in_loop_condition", "a call in a loop condition", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.whl(k.call(g, {}), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}))});
    k.done(f);
  });
  rc("call_in_requires", "a call in a contract", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    k.req(f, k.call(g, {}));
    f->body = k.blk({});
    k.done(f);
  });
  rc("call_in_guarded_operand", "conditionally evaluated operand", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"b", k.bl}});
    f->body = k.blk({k.es(k.land(k.pv(f, 0), k.call(g, {})))});
    k.done(f);
  });
  rc("call_in_conditional_branch", "conditionally evaluated operand", [](Kit& k) {
    Function* g = k.fn("g", k.i32);
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"b", k.bl}});
    f->body = k.blk({k.es(k.cond(k.pv(f, 0), k.call(g, {}), k.num(k.i32, "0")))});
    k.done(f);
  });
  rc("two_unnested_calls", "not nested in each other", [](Kit& k) {
    Function* g = k.fn("g", k.i32);
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.add(k.call(g, {}), k.call(g, {})))});
    k.done(f);
  });
  rc("sibling_calls_in_arguments", "not nested in each other", [](Kit& k) {
    Function* g = k.fn("g", k.i32);
    k.trusted(g);
    Function* h = k.fn("h", k.i32, {{"x", k.i32}, {"y", k.i32}});
    k.trusted(h);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.call(h, {k.call(g, {}), k.call(g, {})}))});
    k.done(f);
  });
  rc("assign_type_mismatch", "where", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.asg(f->param_var(0), k.num(k.u32, "1"))});
    k.done(f);
  });
  rc("decl_type_mismatch", "initializer", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* v = k.local(f, "v", k.i32);
    f->body = k.blk({k.decl(v, k.yes())});
    k.done(f);
  });
  rc("constant_out_of_range", "does not fit", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.num(k.i8, "128"))});
    k.done(f);
  });
  rc("constant_negative_unsigned", "does not fit", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.num(k.u32, "-1"))});
    k.done(f);
  });
  rc("constant_malformed", "malformed", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.num(k.i32, "12a"))});
    k.done(f);
  });
  rc("expression_without_position", "without a source position", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.b.int_const(k.i32, "1", SrcLoc{}))});
    k.done(f);
  });
  rc("void_value_used", "void call is used", [](Kit& k) {
    Function* v = k.fn("v", k.vd);
    k.trusted(v);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.add(k.call(v, {}), k.num(k.i32, "1")))});
    k.done(f);
  });
  rc("return_type_mismatch", "returning", [](Kit& k) {
    Function* f = k.fn("f", k.i32);
    f->body = k.blk({k.ret(k.yes())});
    k.done(f);
  });
  rc("return_value_from_void", "returned from void", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.ret(k.num(k.i32, "1"))});
    k.done(f);
  });
  rc("return_without_value", "`return;`", [](Kit& k) {
    Function* f = k.fn("f", k.i32);
    f->body = k.blk({k.ret(nullptr)});
    k.done(f);
  });
  rc("argument_type_mismatch", "convert it", [](Kit& k) {
    Function* g = k.fn("g", k.vd, {{"x", k.i8}});
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.call(g, {k.num(k.i32, "1")}))});
    k.done(f);
  });
  rc("argument_count", "arguments,", [](Kit& k) {
    Function* g = k.fn("g", k.vd, {{"x", k.i32}});
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.es(k.call(g, {}))});
    k.done(f);
  });
  rc("undeclared_variable", "not declared here", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* v = k.local(f, "v", k.i32);
    f->body = k.blk({k.es(k.v(v))});
    k.done(f);
  });
  rc("variable_declared_twice", "declared twice", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* v = k.local(f, "v", k.i32);
    f->body = k.blk({k.decl(v, k.num(k.i32, "1")), k.decl(v, k.num(k.i32, "2"))});
    k.done(f);
  });
  rc("variable_out_of_scope", "not declared here", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* v = k.local(f, "v", k.i32);
    f->body = k.blk({k.blk({k.decl(v, k.num(k.i32, "1"))}), k.es(k.v(v))});
    k.done(f);
  });
  rc("variable_of_another_function", "does not belong", [](Kit& k) {
    Function* g = k.fn("g", k.vd);
    Var* gv = k.local(g, "gv", k.i32);
    g->body = k.blk({k.decl(gv, k.num(k.i32, "1"))});
    k.done(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.asg(gv, k.num(k.i32, "1"))});
    k.done(f);
  });
  rc("trusted_without_reason", "no reason for trusting", [](Kit& k) { k.fn("f", k.vd); });
  rc("equality_int_bool", "equality operands", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.bl}});
    f->body = k.blk({k.es(k.eq(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  rc("if_condition_is_int", "`if` condition", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.iff(k.pv(f, 0), k.blk({}))});
    k.done(f);
  });
  rc("loop_condition_is_int", "loop condition", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.whl(k.pv(f, 0), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}))});
    k.done(f);
  });
  rc("not_of_int", "not bool", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.es(k.lnot(k.pv(f, 0)))});
    k.done(f);
  });
  rc("logical_and_of_ints", "both must be bool", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.es(k.land(k.pv(f, 0), k.pv(f, 0)))});
    k.done(f);
  });
  rc("conditional_branch_types", "branches", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"b", k.bl}});
    f->body = k.blk({k.es(k.cond(k.pv(f, 0), k.num(k.i32, "1"), k.num(k.u32, "1")))});
    k.done(f);
  });
  rc("convert_bool", "conversion between", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"b", k.bl}});
    f->body = k.blk({k.es(k.cv(k.i32, k.pv(f, 0)))});
    k.done(f);
  });
  rc("to_bool_of_bool", "to_bool of a", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"b", k.bl}});
    f->body = k.blk({k.es(k.tb(k.pv(f, 0)))});
    k.done(f);
  });
  rc("bool_to_int_of_int", "bool_to_int of a", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.es(k.bi(k.i32, k.pv(f, 0)))});
    k.done(f);
  });
  rc("comparison_in_c_must_be_wrapped", "not bool (wrap it", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    // a comparison typed as int, the way C would have it without bool_to_int
    const Expr* c = k.lt(k.pv(f, 0), k.pv(f, 0));
    const_cast<Expr*>(c)->type = k.i32;
    f->body = k.blk({k.es(c)});
    k.done(f);
  });
  rc("requires_not_bool", "is not a bool", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.req(f, k.F(f, 0));
    f->body = k.blk({});
    k.done(f);
  });
  rc("check_not_bool", "must be a bool", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.chk(k.pv(f, 0))});
    k.done(f);
  });
}

}  // namespace
