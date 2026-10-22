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

// ---- control flow (decision 0014): break, continue, do, switch ---------------------------------------------------------

void define_control_flow() {
  using LF = LoopForm;
  // f(n: int) with 0 <= n <= 100.
  auto fn_n = [](Kit& k, Type ret = Type()) {
    Function* f = k.fn("f", ret.is_void() ? k.vd : ret, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    return f;
  };
  auto plus1 = [](Kit& k, Var* x) { return k.asg(x, k.add(k.v(x), k.num(k.i32, "1"))); };

  // ---- break ----
  auto break_search = [=](Kit& k, bool bad_after) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    const int id = k.jid();
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))},
                           {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.iff(k.eq_c(k.v(i), k.i32, "3"), k.blk({k.brk(id)})), plus1(k, i)}), nullptr, LF::While, id);
    const Stmt* after = bad_after ? k.chk(k.eq(k.v(i), k.pv(f, 0))) : k.chk(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 0))));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), w, after, k.ret(k.v(i))});
    k.done(f);
    if (bad_after) k.fail("check", after->loc);
  };
  lc("cf_break_both_exits", "while (i < n) { if (i == 3) break; i++; }: i <= n holds on both exits", [=](Kit& k) { break_search(k, false); });
  lc("cf_break_only_normal_exit_fails", "the same loop: i == n holds only on the normal exit, not after the break", [=](Kit& k) { break_search(k, true); });
  lc("cf_break_skips_invariant_check", "a break does not evaluate the loop condition: the invariant need not hold at the break", [=](Kit& k) {
    // while (i < n) invariant c == 0 { c = 5; break; } : on the break path c is 5, which the invariant forbids; nothing is checked there.
    Function* f = fn_n(k, k.i32);
    Var* c = k.local(f, "c", k.i32);
    Var* i = k.local(f, "i", k.i32);
    const int id = k.jid();
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.eq_c(k.v(c), k.i32, "0"))}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.asg(c, k.num(k.i32, "5")), k.brk(id)}), nullptr, LF::While, id);
    f->body = k.blk({k.decl(c, k.num(k.i32, "0")), k.decl(i, k.num(k.i32, "0")), w, k.chk(k.ge_c(k.v(c), k.i32, "0")), k.ret(k.v(c))});
    k.done(f);
  });

  // ---- continue ----
  auto continue_for = [=](Kit& k, bool bad_after) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* c = k.local(f, "c", k.i32);
    const int id = k.jid();
    const Stmt* body = k.blk({k.iff(k.eq_c(k.v(i), k.i32, "2"), k.blk({k.cnt(id)})), plus1(k, c)});
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)),
                           {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0))), k.cl(k.ge_c(k.v(c), k.i32, "0")), k.cl(k.le(k.v(c), k.v(i)))},
                           {k.cl(k.gap(k.pv(f, 0), k.v(i)))}, body, plus1(k, i), LF::While, id);
    const Stmt* after = bad_after ? k.chk(k.eq(k.v(c), k.pv(f, 0))) : k.chk(k.le(k.v(c), k.pv(f, 0)));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.decl(c, k.num(k.i32, "0")), w, after, k.ret(k.v(c))});
    k.done(f);
    if (bad_after) k.fail("check", after->loc);
  };
  lc("cf_continue_runs_the_step", "for (i = 0; i < n; i++) { if (i == 2) continue; c++; }: the step runs after a continue", [=](Kit& k) { continue_for(k, false); });
  lc("cf_continue_skips_the_rest", "the same loop: c == n does not hold (the continue skips c++)", [=](Kit& k) { continue_for(k, true); });
  lc("cf_continue_without_step_does_not_terminate", "while (i < n) { if (i == 2) continue; i++; }: the continue skips the increment", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    const int id = k.jid();
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100"))}, {m},
                           k.blk({k.iff(k.eq_c(k.v(i), k.i32, "2"), k.blk({k.cnt(id)})), plus1(k, i)}), nullptr, LF::While, id);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), w, k.ret(k.v(i))});
    k.done(f);
    k.fail("termination", m.loc);
  });
  lc("cf_continue_breaks_invariant", "a continue path leaves c == -1, breaking the invariant c == 0", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* c = k.local(f, "c", k.i32);
    const int id = k.jid();
    Clause inv = k.cl(k.eq_c(k.v(c), k.i32, "0"));
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), inv}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.iff(k.eq_c(k.v(i), k.i32, "2"), k.blk({k.asg(c, k.num(k.i32, "-1")), k.cnt(id)}))}), plus1(k, i), LF::While, id);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.decl(c, k.num(k.i32, "0")), w, k.ret(k.v(c))});
    k.done(f);
    k.fail("invariant-preserved", inv.loc);
  });
  lc("cf_continue_condition_obligation", "a continue path reaches the condition 100 / d > 1, whose obligation (d != 0) is an invariant clause", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    Var* d = k.local(f, "d", k.i32);
    const int id = k.jid();
    const Expr* q = k.div(k.num(k.i32, "100"), k.v(d));
    const Stmt* w = k.loop(k.gt(q, k.num(k.i32, "1")), {k.cl(k.range(k.v(d), k.i32, "0", "100"))}, {k.cl(k.sub(k.num(k.i32, "100"), k.v(d)))},
                           k.blk({k.asg(d, k.num(k.i32, "0")), k.cnt(id)}), nullptr, LF::While, id);
    f->body = k.blk({k.decl(d, k.pv(f, 0)), w});
    k.done(f);
    k.fail("division-by-zero", q->loc, 1, "invariant.not.preserved");
  });

  // ---- nested loops ----
  auto nested = [=](Kit& k, bool bad) {
    Function* f = fn_n(k, k.i32);
    Var* s = k.local(f, "s", k.i32);
    Var* j = k.local(f, "j", k.i32);
    const int outer = k.jid();
    const int inner = k.jid();
    const Stmt* in = k.loop(k.lt(k.v(j), k.num(k.i32, "3")), {k.cl(k.range(k.v(j), k.i32, "0", "3"))}, {k.cl(k.sub(k.num(k.i32, "3"), k.v(j)))},
                            k.blk({k.iff(k.eq_c(k.v(j), k.i32, "1"), k.blk({k.brk(inner)})), plus1(k, j)}), nullptr, LF::While, inner);
    const Stmt* fact = bad ? k.chk(k.eq_c(k.v(j), k.i32, "3")) : k.chk(k.lor(k.eq_c(k.v(j), k.i32, "1"), k.eq_c(k.v(j), k.i32, "3")));
    // while (s < n) { j = 0; while (j < 3) { if (j == 1) break; j++; } check(...); s++; if (s == 5) continue; }  -- the continue is last
    const Stmt* out = k.loop(k.lt(k.v(s), k.pv(f, 0)), {k.cl(k.range(k.v(s), k.i32, "0", "100"))}, {k.cl(k.gap(k.pv(f, 0), k.v(s)))},
                             k.blk({k.asg(j, k.num(k.i32, "0")), in, fact, plus1(k, s), k.iff(k.eq_c(k.v(s), k.i32, "5"), k.blk({k.cnt(outer)}))}),
                             nullptr, LF::While, outer);
    f->body = k.blk({k.decl(s, k.num(k.i32, "0")), k.decl(j, k.num(k.i32, "0")), out, k.ret(k.v(s))});
    k.done(f);
    if (bad) k.fail("check", fact->loc);
  };
  lc("cf_nested_break_inner_only", "a break leaves the inner loop only; after it j == 1 or j == 3; the outer loop goes on", [=](Kit& k) { nested(k, false); });
  lc("cf_nested_break_fact_too_strong", "the same loops: j == 3 does not hold after the inner loop", [=](Kit& k) { nested(k, true); });

  // ---- do ... while ----
  // do { i++; } while (i < n);  with the invariant "i == 0 || i < n" and the measure n - i + 1, returning i.
  auto do_count = [=](Kit& k, int variant) {
    Function* f = fn_n(k, k.i32);
    if (variant == 0 || variant == 4) k.ens(f, k.eq(k.R(f), k.cond(k.lt(k.F(f, 0), k.num(k.i32, "1")), k.num(k.i32, "1"), k.F(f, 0))));
    Var* i = k.local(f, "i", k.i32);
    const int id = k.jid();
    Clause inv = k.cl(variant == 1 ? k.eq_c(k.v(i), k.i32, "1") : variant == 2 ? k.le_c(k.v(i), k.i32, "3")
                                                                               : k.lor(k.eq_c(k.v(i), k.i32, "0"), k.lt(k.v(i), k.pv(f, 0))));
    Clause m = k.cl(variant == 3 ? k.add(k.cv(k.i64, k.pv(f, 0)), k.num(k.i64, "1"))
                    : variant == 4 ? k.sub(k.sub(k.cv(k.i64, k.pv(f, 0)), k.cv(k.i64, k.v(i))), k.num(k.i64, "1"))
                                   : k.add(k.sub(k.cv(k.i64, k.pv(f, 0)), k.cv(k.i64, k.v(i))), k.num(k.i64, "1")));
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), inv}, {m}, k.blk({plus1(k, i)}), nullptr, LF::DoWhile, id);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), w, k.ret(k.v(i))});
    k.done(f);
    if (variant == 1) k.fail("invariant-entry", inv.loc);
    if (variant == 2) k.fail("invariant-preserved", inv.loc);
    if (variant == 3) k.fail("termination", m.loc);
  };
  lc("cf_do_count_ok", "do { i++; } while (i < n) with the invariant i == 0 || i < n: returns max(n, 1)", [=](Kit& k) { do_count(k, 0); });
  lc("cf_do_invariant_entry_fails", "the do loop with the invariant i == 1: it does not hold at the start of the first iteration", [=](Kit& k) { do_count(k, 1); });
  lc("cf_do_invariant_not_preserved", "the do loop with the invariant i <= 3: it does not hold at the start of a later iteration", [=](Kit& k) { do_count(k, 2); });
  lc("cf_do_measure_does_not_decrease", "the do loop with the constant measure n + 1", [=](Kit& k) { do_count(k, 3); });
  lc("cf_do_negative_measure_in_last_iteration_ok", "the measure n - i - 1 is -1 in the only iteration when n == 0: accepted, no iteration follows", [=](Kit& k) { do_count(k, 4); });

  // do { i++; if (i == 2) continue; c++; if (i == 3) break; } while (i < n);
  auto do_continue = [=](Kit& k, bool bad_invariant) {
    Function* f = fn_n(k, k.vd);
    Var* i = k.local(f, "i", k.i32);
    Var* c = k.local(f, "c", k.i32);
    const int id = k.jid();
    Clause inv = k.cl(bad_invariant ? k.eq(k.v(c), k.v(i)) : k.land(k.ge_c(k.v(c), k.i32, "0"), k.le(k.v(c), k.v(i))));
    Clause m = k.cl(k.add(k.sub(k.cv(k.i64, k.pv(f, 0)), k.cv(k.i64, k.v(i))), k.num(k.i64, "1")));
    const Stmt* body = k.blk({plus1(k, i), k.iff(k.eq_c(k.v(i), k.i32, "2"), k.blk({k.cnt(id)})), plus1(k, c), k.iff(k.eq_c(k.v(i), k.i32, "3"), k.blk({k.brk(id)}))});
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.lor(k.eq_c(k.v(i), k.i32, "0"), k.lt(k.v(i), k.pv(f, 0)))), inv}, {m}, body, nullptr, LF::DoWhile, id);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.decl(c, k.num(k.i32, "0")), w, k.chk(k.ge_c(k.v(i), k.i32, "1"))});
    k.done(f);
    if (bad_invariant) k.fail("invariant-preserved", inv.loc);
  };
  lc("cf_do_continue_and_break", "continue in a do loop goes to the condition (invariant checked there); break leaves without a check", [=](Kit& k) { do_continue(k, false); });
  lc("cf_do_continue_breaks_invariant", "the continue path skips c++, so c == i does not hold at the next iteration", [=](Kit& k) { do_continue(k, true); });

  auto do_cond = [=](Kit& k, bool unguarded) {
    // d = 0 (or n); do { d = 1; } while (100 / d > 200): the condition is evaluated after the body, never on entry.
    Function* f = fn_n(k, k.vd);
    Var* d = k.local(f, "d", k.i32);
    const int id = k.jid();
    const Expr* q = k.div(k.num(k.i32, "100"), k.v(d));
    const Stmt* w = k.loop(k.gt(q, k.num(k.i32, "200")), {k.cl(k.range(k.v(d), k.i32, "0", "100"))}, {k.cl(k.num(k.i32, "0"))},
                           k.blk({k.asg(d, unguarded ? k.pv(f, 0) : k.num(k.i32, "1"))}), nullptr, LF::DoWhile, id);
    f->body = k.blk({k.decl(d, k.num(k.i32, "0")), w});
    k.done(f);
    if (unguarded) k.fail("division-by-zero", q->loc);
  };
  lc("cf_do_condition_obligation_after_body", "do { d = 1; } while (100 / d > 200) with d == 0 on entry: the division is checked after the body", [=](Kit& k) { do_cond(k, false); });
  lc("cf_do_condition_obligation_fails", "do { d = n; } while (100 / d > 200): n may be 0 when the condition is evaluated", [=](Kit& k) { do_cond(k, true); });

  // ---- do ... while (0): no loop ----
  // do { i += 1; if (n == 7) continue; i += 10; if (n == 8) break; i += 100; } while (0);
  auto once = [=](Kit& k, int variant) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    const int id = k.jid();
    auto add_to_i = [&](const char* v) { return k.asg(i, k.add(k.v(i), k.num(k.i32, v))); };
    const Stmt* bad_check = variant == 2 ? k.chk(k.eq_c(k.v(i), k.i32, "5")) : k.chk(k.ge_c(k.v(i), k.i32, "0"));
    const Stmt* body = k.blk({add_to_i("1"), k.iff(k.eq_c(k.pv(f, 0), k.i32, "7"), k.blk({k.cnt(id)})), add_to_i("10"),
                              k.iff(k.eq_c(k.pv(f, 0), k.i32, "8"), k.blk({k.brk(id)})), add_to_i("100"), bad_check});
    Clause inv = k.cl(variant == 3 ? k.eq_c(k.v(i), k.i32, "1") : k.eq_c(k.v(i), k.i32, "0"));
    const Stmt* w = k.loop(k.no(), {inv}, {}, body, nullptr, LF::DoOnce, id);
    const Stmt* r = k.ret(k.v(i));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), w, r});
    k.done(f);
    const Expr* want = variant == 1 ? k.eq_c(k.R(f), k.i32, "2")
                                    : k.land(k.implies(k.eq_c(k.F(f, 0), k.i32, "7"), k.eq_c(k.R(f), k.i32, "1")),
                                             k.land(k.implies(k.eq_c(k.F(f, 0), k.i32, "8"), k.eq_c(k.R(f), k.i32, "11")),
                                                    k.implies(k.land(k.ne(k.F(f, 0), k.num(k.i32, "7")), k.ne(k.F(f, 0), k.num(k.i32, "8"))), k.eq_c(k.R(f), k.i32, "111"))));
    k.ens(f, want);
    if (variant == 1) k.fail("postcondition", r->loc);
    if (variant == 2) k.fail("check", bad_check->loc);
    if (variant == 3) k.fail("invariant-entry", inv.loc);
  };
  lc("cf_do_once_continue_and_break", "do { ... } while (0) with continue and break: no Viper loop, nothing about i is forgotten", [=](Kit& k) { once(k, 0); });
  lc("cf_do_once_wrong_postcondition", "the same, with a wrong postcondition for n == 7", [=](Kit& k) { once(k, 1); });
  lc("cf_do_once_check_fails", "a check inside the do-once body that does not always hold", [=](Kit& k) { once(k, 2); });
  lc("cf_do_once_invariant_entry_fails", "a written invariant of a do-once loop is asserted at entry", [=](Kit& k) { once(k, 3); });
  lc("cf_do_once_measure_obligation", "a written measure's own arithmetic (100 / n) is checked at entry", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"n", k.i32}});
    const Expr* q = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    const int id = k.jid();
    f->body = k.blk({k.loop(k.no(), {}, {k.cl(q)}, k.blk({}), nullptr, LF::DoOnce, id)});
    k.done(f);
    k.fail("division-by-zero", q->loc);
  });
  lc("cf_do_once_has_an_internal_backstop", "a do-once loop asserts that its condition is false, as an internal check", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const int id = k.jid();
    f->body = k.blk({k.loop(k.no(), {}, {}, k.blk({}), nullptr, LF::DoOnce, id)});
    k.done(f);
    k.require_map("internal", "the condition of a do-once loop is not false");
  });
  lc("cf_do_once_condition_conversion_fails", "do { } while ((signed char)256): EDG folds it to 0; the conversion obligation is what the user sees", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* conv = k.cv(k.i8, k.num(k.i32, "256"));
    const int id = k.jid();
    f->body = k.blk({k.loop(k.tb(conv), {}, {}, k.blk({}), nullptr, LF::DoOnce, id)});
    k.done(f);
    k.fail("conversion", conv->loc);
  });
  lc("cf_do_once_condition_shift_fails", "do { } while (1 << 40): the shift obligation of the condition fails, and nothing else", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* sh = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    const int id = k.jid();
    f->body = k.blk({k.loop(k.tb(sh), {}, {}, k.blk({}), nullptr, LF::DoOnce, id)});
    k.done(f);
    k.fail("shift", sh->loc, 1, "", "shift count");
  });
  lc("cf_do_once_constant_expression", "do { } while (1 - 1): the condition is a constant zero by evaluation", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* c = k.tb(k.sub(k.num(k.i32, "1"), k.num(k.i32, "1")));
    const int id = k.jid();
    f->body = k.blk({k.loop(c, {}, {}, k.blk({}), nullptr, LF::DoOnce, id)});
    k.done(f);
  });

  // ---- loops left only by break ----
  auto forever = [=](Kit& k, bool terminates) {
    Function* f = fn_n(k, k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), k.num(k.i32, "1"))));
    Var* i = k.local(f, "i", k.i32);
    const int id = k.jid();
    Clause m = k.cl(terminates ? k.sub(k.add(k.cv(k.i64, k.pv(f, 0)), k.num(k.i64, "1")), k.cv(k.i64, k.v(i))) : k.num(k.i64, "100"));
    const Stmt* w = k.loop(k.yes(), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))}, {m},
                           k.blk({plus1(k, i), k.iff(k.gt(k.v(i), k.pv(f, 0)), k.blk({k.brk(id)}))}), nullptr, LF::While, id);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), w, k.ret(k.v(i))});
    k.done(f);
    if (!terminates) k.fail("termination", m.loc);
  };
  lc("cf_forever_left_by_break", "for (;;) { i++; if (i > n) break; }: returns n + 1", [=](Kit& k) { forever(k, true); });
  lc("cf_forever_measure_fails", "the same loop with the constant measure 100: the measure does not decrease", [=](Kit& k) { forever(k, false); });
  lc("cf_forever_without_exit_never_returns", "while (1) with no exit and no return: the code after it is unreachable, so no missing-return", [](Kit& k) {
    Function* f = k.fn("f", k.i32);
    const int id = k.jid();
    // while (true) invariant true decreases 0 {}  -- Silicon does not report the missing return after a loop that cannot be left
    f->body = k.blk({k.loop(k.yes(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), nullptr, LF::While, id)});
    k.done(f);
    k.fail("termination", f->body->stmts[0]->measure[0].loc);
  });

  // ---- switch ----
  // switch (x) { case 1: r = 10; case 2: r = r + 1; break; default: r = 7; case 5: r = r + 100; }  (default in the middle)
  auto dispatch = [=](Kit& k, bool bad) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "-100", "100"));
    Var* r = k.local(f, "r", k.i32);
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.num(k.i32, "1")), k.asg(r, k.num(k.i32, "10")), k.cs(k.num(k.i32, "2")), k.asg(r, k.add(k.v(r), k.num(k.i32, "1"))),
                              k.brk(id), k.dflt(), k.asg(r, k.num(k.i32, "7")), k.cs(k.num(k.i32, "5")), k.asg(r, k.add(k.v(r), k.num(k.i32, "100")))});
    const Stmt* s = k.sw(k.pv(f, 0), body, id);
    const Stmt* rt = k.ret(k.v(r));
    f->body = k.blk({k.decl(r, k.num(k.i32, "0")), s, rt});
    k.done(f);
    auto x = [&](const char* v) { return k.eq_c(k.F(f, 0), k.i32, v); };
    if (!bad) {
      k.ens(f, k.implies(x("1"), k.eq_c(k.R(f), k.i32, "11")));
      k.ens(f, k.implies(x("2"), k.eq_c(k.R(f), k.i32, "1")));
      k.ens(f, k.implies(x("5"), k.eq_c(k.R(f), k.i32, "100")));
    }
    k.ens(f, k.implies(k.land(k.ne(k.F(f, 0), k.num(k.i32, "1")), k.land(k.ne(k.F(f, 0), k.num(k.i32, "2")), k.ne(k.F(f, 0), k.num(k.i32, "5")))),
                       k.eq_c(k.R(f), k.i32, bad ? "7" : "107")));
    if (bad) k.fail("postcondition", rt->loc);
  };
  lc("cf_switch_dispatch_fallthrough_default_in_middle", "case 1 falls into case 2; default (in the middle) falls into case 5", [=](Kit& k) { dispatch(k, false); });
  lc("cf_switch_default_falls_into_next_case", "the postcondition forgets that default falls through into case 5", [=](Kit& k) { dispatch(k, true); });
  lc("cf_switch_no_default_matches_nothing", "no default: a value that matches no case skips the body", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    Var* r = k.local(f, "r", k.i32);
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.num(k.i32, "1")), k.asg(r, k.num(k.i32, "10")), k.cs(k.num(k.i32, "2")), k.asg(r, k.num(k.i32, "20"))});
    f->body = k.blk({k.decl(r, k.num(k.i32, "0")), k.sw(k.pv(f, 0), body, id), k.ret(k.v(r))});
    k.done(f);
    k.ens(f, k.implies(k.land(k.ne(k.F(f, 0), k.num(k.i32, "1")), k.ne(k.F(f, 0), k.num(k.i32, "2"))), k.eq_c(k.R(f), k.i32, "0")));
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "2"), k.eq_c(k.R(f), k.i32, "20")));
  });
  lc("cf_switch_missing_return_without_default", "every case returns but there is no default: control may reach the end", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.num(k.i32, "1")), k.ret(k.num(k.i32, "1")), k.cs(k.num(k.i32, "2")), k.ret(k.num(k.i32, "2"))});
    f->body = k.blk({k.sw(k.pv(f, 0), body, id)});
    k.done(f);
    k.fail("missing-return", f->end_loc);
  });
  lc("cf_switch_every_path_returns", "every case and the default return: no missing return", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.num(k.i32, "1")), k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "2"))});
    f->body = k.blk({k.sw(k.pv(f, 0), body, id)});
    k.done(f);
  });
  lc("cf_switch_scrutinee_obligation", "switch (100 / a): the division is checked before the dispatch", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    const Expr* q = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    const int id = k.jid();
    f->body = k.blk({k.sw(q, k.blk({k.cs(k.num(k.i32, "1")), k.brk(id)}), id)});
    k.done(f);
    k.fail("division-by-zero", q->loc);
  });
  lc("cf_switch_on_a_call_evaluates_it_once", "switch (g(a)): g's precondition is checked at the call, its postcondition decides the case", [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.req(g, k.gt(k.F(g, 0), k.num(k.i32, "0")));
    k.ens(g, k.eq_c(k.R(g), k.i32, "1"));
    k.trusted(g);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.num(k.i32, "1")), k.ret(k.num(k.i32, "5")), k.dflt(), k.ret(k.num(k.i32, "6"))});
    f->body = k.blk({k.sw(k.call(g, {k.pv(f, 0)}), body, id)});
    k.done(f);
    k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
  });
  lc("cf_switch_on_a_call_precondition_fails", "switch (g(a)) with a unconstrained: g's precondition may not hold", [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.req(g, k.gt(k.F(g, 0), k.num(k.i32, "0")));
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    const Expr* c = k.call(g, {k.pv(f, 0)});
    const int id = k.jid();
    f->body = k.blk({k.sw(c, k.blk({k.cs(k.num(k.i32, "1")), k.brk(id)}), id)});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  lc("cf_switch_case_value_obligation_fails", "case 1 << 40 on an int: the shift obligation is asserted at the case label, before the dispatch", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* bad = k.cs(k.shl(k.num(k.i32, "1"), k.num(k.i32, "40")), "0");  // EDG folds 1 << 40 to 0
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "2")), k.ret(k.num(k.i32, "1")), bad, k.ret(k.num(k.i32, "2"))}), id), k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail("shift", bad->loc, 1, "", "shift count");  // the later obligations of the same case value hold vacuously after it
  });
  lc("cf_switch_case_value_expression_ok", "case 1 << 3 and case (2 + 3): obligations hold, values used in the dispatch", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.shl(k.num(k.i32, "1"), k.num(k.i32, "3")), "8"), k.ret(k.num(k.i32, "8")), k.cs(k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), "5"),
                              k.ret(k.num(k.i32, "5"))});
    f->body = k.blk({k.sw(k.pv(f, 0), body, id), k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "8"), k.eq_c(k.R(f), k.i32, "8")));
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "5"), k.eq_c(k.R(f), k.i32, "5")));
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "3"), k.eq_c(k.R(f), k.i32, "0")));
  });
  lc("cf_switch_scrutinee_obligation_after_the_call", "switch (100 / g(x)): the divisor check comes after the call, so g's result is known", [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.ens(g, k.range(k.R(g), k.i32, "1", "10"));
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    const int id = k.jid();
    f->body = k.blk({k.sw(k.div(k.num(k.i32, "100"), k.call(g, {k.pv(f, 0)})), k.blk({k.cs(k.num(k.i32, "10")), k.brk(id)}), id)});
    k.done(f);
  });
  lc("cf_switch_scrutinee_obligation_after_the_call_fails", "switch (100 / g(x)) where g may return 0: the check after the call fails", [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.ens(g, k.range(k.R(g), k.i32, "0", "10"));
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    const int id = k.jid();
    const Expr* q = k.div(k.num(k.i32, "100"), k.call(g, {k.pv(f, 0)}));
    f->body = k.blk({k.sw(q, k.blk({k.cs(k.num(k.i32, "10")), k.brk(id)}), id)});
    k.done(f);
    k.fail("division-by-zero", q->loc);
  });
  lc("cf_switch_unsigned_scrutinee", "switch on an unsigned int with the case value 4294967295 (what -1 becomes)", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"u", k.u32}});
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.num(k.u32, "4294967295")), k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))});
    f->body = k.blk({k.sw(k.pv(f, 0), body, id)});
    k.done(f);
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.u32, "4294967295"), k.eq_c(k.R(f), k.i32, "1")));
    k.ens(f, k.implies(k.ne(k.F(f, 0), k.num(k.u32, "4294967295")), k.eq_c(k.R(f), k.i32, "0")));
  });
  // while (s < n) { switch (s) { case 1: s += 1; break; case 2: s += 3; break; default: s += 1; } s += 1; }
  auto switch_in_loop = [=](Kit& k, bool bad) {
    Function* f = fn_n(k, k.i32);
    Var* s = k.local(f, "s", k.i32);
    const int lid = k.jid();
    const int sid = k.jid();
    auto add_s = [&](const char* v) { return k.asg(s, k.add(k.v(s), k.num(k.i32, v))); };
    const Stmt* swt = k.sw(k.v(s), k.blk({k.cs(k.num(k.i32, "1")), add_s("1"), k.brk(sid), k.cs(k.num(k.i32, "2")), add_s("3"), k.brk(sid), k.dflt(), add_s("1")}), sid);
    const Stmt* w = k.loop(k.lt(k.v(s), k.pv(f, 0)), {k.cl(k.range(k.v(s), k.i32, "0", "110"))}, {k.cl(k.sub(k.num(k.i64, "110"), k.cv(k.i64, k.v(s))))},
                           k.blk({swt, add_s("1")}), nullptr, LF::While, lid);
    const Stmt* after = bad ? k.chk(k.eq(k.v(s), k.pv(f, 0))) : k.chk(k.ge(k.v(s), k.pv(f, 0)));
    f->body = k.blk({k.decl(s, k.num(k.i32, "0")), w, after, k.ret(k.v(s))});
    k.done(f);
    if (bad) k.fail("check", after->loc);
  };
  lc("cf_switch_break_inside_loop", "a switch inside a loop: break leaves the switch only, then s += 1 runs", [=](Kit& k) { switch_in_loop(k, false); });
  lc("cf_switch_break_inside_loop_overshoots", "the same loop: s == n does not hold (the body can overshoot)", [=](Kit& k) { switch_in_loop(k, true); });
  // for (i = 0; i < n; i++) { switch (i) { case 1: s = 1; continue; case 2: s = 5; break; default: s = 2; } s = 100; }
  auto switch_continue = [=](Kit& k, bool step_in_body) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* s = k.local(f, "s", k.i32);
    const int lid = k.jid();
    const int sid = k.jid();
    auto add_s = [&](const char* v) { return k.asg(s, k.num(k.i32, v)); };
    const Stmt* swt = k.sw(k.v(i), k.blk({k.cs(k.num(k.i32, "1")), add_s("1"), k.cnt(lid), k.cs(k.num(k.i32, "2")), add_s("5"), k.brk(sid), k.dflt(), add_s("2")}), sid);
    std::vector<const Stmt*> body = {swt, add_s("100")};
    if (step_in_body) body.push_back(plus1(k, i));
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)),
                           {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0))), k.cl(k.ge_c(k.v(s), k.i32, "0"))},
                           {m}, k.blk(body), step_in_body ? nullptr : plus1(k, i), LF::While, lid);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.decl(s, k.num(k.i32, "0")), w, k.chk(k.ge_c(k.v(s), k.i32, "0")), k.ret(k.v(s))});
    k.done(f);
    if (step_in_body) k.fail("termination", m.loc);
  };
  lc("cf_switch_continue_runs_the_step", "a continue inside a switch continues the loop: the for step runs", [=](Kit& k) { switch_continue(k, false); });
  lc("cf_switch_continue_skips_body_increment", "the increment is the last statement of the body, so a continue in the switch skips it", [=](Kit& k) { switch_continue(k, true); });
  lc("cf_auto_range_invariant_from_switch_and_step", "a signed char assigned only in a switch and a for step still gets its automatic range invariant", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* c = k.local(f, "c", k.i8);
    Var* t = k.local(f, "t", k.i32);
    const int lid = k.jid();
    const int sid = k.jid();
    const Expr* inc = k.cv(k.i8, k.add(k.cv(k.i32, k.v(c)), k.num(k.i32, "1")));
    const Stmt* swt = k.sw(k.v(t), k.blk({k.cs(k.num(k.i32, "0")), k.asg(c, inc), k.brk(sid)}), sid);
    const Stmt* w = k.loop(k.lt(k.v(c), k.num(k.i8, "10")), {}, {k.cl(k.sub(k.num(k.i32, "10"), k.cv(k.i32, k.v(c))))}, k.blk({swt}), k.asg(c, inc), LF::While, lid);
    f->body = k.blk({k.decl(c, k.num(k.i8, "0")), k.decl(t, k.num(k.i32, "0")), w});
    k.done(f);
  });

  // ---- what the checker refuses ----
  auto in_fn = [](Kit& k, const std::function<const Stmt*(Kit&, Function*)>& body) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.bl}});
    f->body = k.blk({body(k, f)});
    k.done(f);
  };
  rc("cf_break_outside_loop", "outside any loop or switch", [in_fn](Kit& k) { in_fn(k, [](Kit& k, Function*) { return k.brk(k.jid()); }); });
  rc("cf_continue_outside_loop", "outside any loop", [in_fn](Kit& k) { in_fn(k, [](Kit& k, Function*) { return k.cnt(k.jid()); }); });
  rc("cf_break_names_a_sibling_loop", "does not name the innermost enclosing loop or switch", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) {
      const int a = k.jid();
      const int b = k.jid();
      return k.blk({k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), nullptr, LoopForm::While, a),
                    k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.brk(a)}), nullptr, LoopForm::While, b)});
    });
  });
  rc("cf_break_with_id_zero", "does not name the innermost enclosing loop or switch", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) { return k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.brk(0)}), nullptr, LoopForm::While, 0); });
  });
  rc("cf_continue_names_a_switch", "does not name the innermost enclosing loop", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {
      const int l = k.jid();
      const int s = k.jid();
      return k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "1")), k.cnt(s)}), s)}), nullptr, LoopForm::While, l);
    });
  });
  rc("cf_duplicate_jump_id", "used by two loops or switches", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) {
      const int a = k.jid();
      return k.blk({k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), nullptr, LoopForm::While, a),
                    k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), nullptr, LoopForm::While, a)});
    });
  });
  rc("cf_break_names_an_outer_loop", "does not name the innermost enclosing loop or switch", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) {
      const int outer = k.jid();
      const int inner = k.jid();
      return k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))},
                    k.blk({k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.brk(outer)}), nullptr, LoopForm::While, inner)}), nullptr,
                    LoopForm::While, outer);
    });
  });
  rc("cf_break_in_a_switch_names_the_loop", "does not name the innermost enclosing loop or switch", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {
      const int l = k.jid();
      const int s = k.jid();
      return k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "1")), k.brk(l)}), s)}), nullptr,
                    LoopForm::While, l);
    });
  });
  rc("cf_continue_names_an_outer_loop", "does not name the innermost enclosing loop", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) {
      const int outer = k.jid();
      const int inner = k.jid();
      return k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))},
                    k.blk({k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.cnt(outer)}), nullptr, LoopForm::While, inner)}), nullptr,
                    LoopForm::While, outer);
    });
  });
  rc("cf_case_value_with_a_call", "a call in a loop condition or in a `check`", [](Kit& k) {
    Function* g = k.fn("g", k.i32);
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.call(g, {}))}), k.jid())});
    k.done(f);
  });
  rc("cf_case_outside_switch", "not directly in the body of its switch", [in_fn](Kit& k) { in_fn(k, [](Kit& k, Function*) { return k.cs(k.num(k.i32, "1")); }); });
  rc("cf_case_in_nested_block", "not directly in the body of its switch", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {
      const int s = k.jid();
      return k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "0")), k.blk({k.cs(k.num(k.i32, "1"))})}), s);
    });
  });
  rc("cf_case_inside_a_loop_in_the_switch", "not directly in the body of its switch", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {  // Duff's device
      const int s = k.jid();
      const int l = k.jid();
      return k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "0")), k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.cs(k.num(k.i32, "1"))}), nullptr, LoopForm::While, l)}), s);
    });
  });
  rc("cf_duplicate_case_value", "appears twice", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {
      return k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "1")), k.cs(k.num(k.i32, "1"))}), k.jid());
    });
  });
  rc("cf_two_defaults", "second `default`", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 0), k.blk({k.dflt(), k.dflt()}), k.jid()); });
  });
  rc("cf_case_value_of_the_wrong_type", "a case value of type", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.u32, "1"))}), k.jid()); });
  });
  rc("cf_case_value_not_a_constant", "not a constant expression", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 0), k.blk({k.cs(k.pv(f, 0))}), k.jid()); });
  });
  rc("cf_declaration_jumped_over", "jumps over", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {
      Var* x = k.local(f, "x", k.i32);
      return k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "0")), k.decl(x, k.num(k.i32, "5")), k.cs(k.num(k.i32, "1"))}), k.jid());
    });
  });
  rc("cf_statement_before_first_case", "before the first case label", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 0), k.blk({k.chk(k.yes()), k.cs(k.num(k.i32, "1"))}), k.jid()); });
  });
  rc("cf_switch_on_bool", "a switch on a bool", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 1), k.blk({}), k.jid()); });
  });
  rc("cf_switch_without_id", "a switch without a jump id", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 0), k.blk({}), 0); });
  });
  rc("cf_switch_body_not_a_block", "not a compound statement", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.sw(k.pv(f, 0), k.chk(k.yes()), k.jid()); });
  });
  rc("cf_call_in_do_condition", "a call in a loop condition", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.loop(k.call(g, {}), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), nullptr, LoopForm::DoWhile, k.jid())});
    k.done(f);
  });
  rc("cf_call_in_do_once_condition", "a call in a loop condition", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    k.trusted(g);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.loop(k.call(g, {}), {}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid())});
    k.done(f);
  });
  rc("cf_do_without_measure", "without a termination measure", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.loop(k.lt(k.pv(f, 0), k.num(k.i32, "5")), {}, {}, k.blk({}), nullptr, LoopForm::DoWhile, k.jid()); });
  });
  rc("cf_while_without_measure_still_rejected", "without a termination measure", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.loop(k.lt(k.pv(f, 0), k.num(k.i32, "5")), {}, {}, k.blk({}), nullptr, LoopForm::While, k.jid()); });
  });
  rc("cf_do_without_jump_id", "a `do` loop without a jump id", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.loop(k.lt(k.pv(f, 0), k.num(k.i32, "5")), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), nullptr, LoopForm::DoWhile, 0); });
  });
  rc("cf_do_once_with_a_live_condition", "not a constant zero", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) { return k.loop(k.lt(k.pv(f, 0), k.num(k.i32, "5")), {}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid()); });
  });
  rc("cf_do_once_with_a_true_condition", "not a constant zero", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) { return k.loop(k.yes(), {}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid()); });
  });
  rc("cf_loop_step_not_an_assignment", "not an assignment or an expression statement", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function*) { return k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), k.iff(k.yes(), k.blk({})), LoopForm::While, k.jid()); });
  });
  rc("cf_step_on_a_do_loop", "a loop step on a `do` loop", [in_fn](Kit& k) {
    in_fn(k, [](Kit& k, Function* f) {
      Var* x = k.local(f, "x", k.i32);
      return k.blk({k.decl(x, k.num(k.i32, "0")),
                    k.loop(k.no(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({}), k.asg(x, k.num(k.i32, "1")), LoopForm::DoWhile, k.jid())});
    });
  });
}

// ---- user gotos, functions that do not return, assert (decision 0015) -----------------------------------------------

void define_goto_noreturn_assert() {
  using LF = LoopForm;
  auto fn_n = [](Kit& k, Type ret = Type()) {
    Function* f = k.fn("f", ret.is_void() ? k.vd : ret, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    return f;
  };
  auto plus1 = [](Kit& k, Var* x) { return k.asg(x, k.add(k.v(x), k.num(k.i32, "1"))); };

  // ---- goto ----
  // S11a: goto from an inner loop to a label after the outer loop.  The twin checks a fact true only on the normal exit.
  auto nested = [=](Kit& k, bool twin) {
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* j = k.local(f, "j", k.i32);
    Var* r = k.local(f, "r", k.i32);
    const int L = k.jid();
    auto inv = [&](Var* x) {
      return std::vector<Clause>{k.cl(k.range(k.v(x), k.i32, "0", "100")), k.cl(k.le(k.v(x), k.pv(f, 0))), k.cl(k.range(k.v(r), k.i32, "0", "2"))};
    };
    const Stmt* inner = k.loop(k.lt(k.v(j), k.pv(f, 0)), inv(j), {k.cl(k.gap(k.pv(f, 0), k.v(j)))},
                               k.blk({k.iff(k.eq_c(k.add(k.v(i), k.v(j)), k.i32, "7"), k.blk({k.asg(r, k.num(k.i32, "1")), k.go(L)})), plus1(k, j)}),
                               nullptr, LF::While, k.jid());
    const Stmt* outer = k.loop(k.lt(k.v(i), k.pv(f, 0)), inv(i), {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                               k.blk({k.asg(j, k.num(k.i32, "0")), inner, plus1(k, i)}), nullptr, LF::While, k.jid());
    const Stmt* c = twin ? k.chk(k.eq_c(k.v(r), k.i32, "2")) : k.chk(k.ge_c(k.v(r), k.i32, "1"));
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.decl(r, k.num(k.i32, "0")), k.decl(j, k.num(k.i32, "0")), outer,
                     k.asg(r, k.num(k.i32, "2")), k.lab(L), c, k.ret(k.v(r))});
    k.done(f);
    if (twin) k.fail("check", c->loc);
  };
  lc("goto_out_of_nested_loops", "goto from the inner loop to a label after the outer loop: r >= 1 on both exits", [=](Kit& k) { nested(k, false); });
  lc("goto_out_of_nested_loops_twin", "the same: r == 2 holds only on the normal exit, not after the goto", [=](Kit& k) { nested(k, true); });

  // S11b: a continue-like goto to a later label of a loop body.
  auto cont_like = [=](Kit& k, int variant) {  // 0 ok, 1 breaks an invariant, 2 skips the step
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* r = k.local(f, "r", k.i32);
    const int L = k.jid();
    Clause inv = k.cl(k.land(k.ge_c(k.v(r), k.i32, "0"), k.le(k.v(r), k.v(i))));
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    const Stmt* jump = variant == 1 ? k.blk({k.asg(r, k.add(k.v(r), k.num(k.i32, "5"))), k.go(L)}) : k.blk({k.go(L)});
    std::vector<const Stmt*> body = {k.iff(k.eq_c(k.v(i), k.i32, "3"), jump), k.asg(r, k.add(k.v(r), k.num(k.i32, "1")))};
    if (variant == 2) {
      body.push_back(plus1(k, i));
      body.push_back(k.lab(L));
    } else {
      body.push_back(k.lab(L));
      body.push_back(plus1(k, i));
    }
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0))), inv}, {m},
                           k.blk(body), nullptr, LF::While, k.jid());
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), k.decl(r, k.num(k.i32, "0")), w, k.ret(k.v(r))});
    k.done(f);
    if (variant == 1) k.fail("invariant-preserved", inv.loc);
    if (variant == 2) k.fail("termination", m.loc);
  };
  lc("goto_continue_like", "a goto to a later label of the loop body (like continue): the invariants hold at the end of the body", [=](Kit& k) { cont_like(k, 0); });
  lc("goto_continue_like_breaks_invariant", "the goto path adds 5 to r, so r <= i does not hold at the next iteration", [=](Kit& k) { cont_like(k, 1); });
  lc("goto_continue_like_skips_step", "the label is after the step: the goto path does not decrease the measure", [=](Kit& k) { cont_like(k, 2); });

  // S11c: goto out of a do ... while (0) (straight-line code).
  auto from_once = [=](Kit& k, bool twin) {
    Function* f = fn_n(k, k.i32);
    Var* r = k.local(f, "r", k.i32);
    const int L = k.jid();
    const Stmt* body = k.blk({k.iff(k.eq_c(k.pv(f, 0), k.i32, "5"), k.blk({k.asg(r, k.num(k.i32, "1")), k.go(L)})), k.asg(r, k.num(k.i32, "1"))});
    const Stmt* c = twin ? k.chk(k.eq_c(k.v(r), k.i32, "2")) : k.chk(k.ge_c(k.v(r), k.i32, "1"));
    f->body = k.blk({k.decl(r, k.num(k.i32, "0")), k.loop(k.no(), {}, {}, body, nullptr, LF::DoOnce, k.jid()),
                     k.asg(r, k.add(k.v(r), k.num(k.i32, "1"))), k.lab(L), c, k.ret(k.v(r))});
    k.done(f);
    if (twin) k.fail("check", c->loc);
  };
  lc("goto_out_of_do_once", "goto out of do { ... } while (0) to a later label: r >= 1 on both paths", [=](Kit& k) { from_once(k, false); });
  lc("goto_out_of_do_once_twin", "the same: r == 2 holds only when the body ran to its end", [=](Kit& k) { from_once(k, true); });

  // goto out of a switch.
  auto from_switch = [=](Kit& k, bool twin) {
    Function* f = fn_n(k, k.i32);
    Var* r = k.local(f, "r", k.i32);
    const int L = k.jid();
    const Stmt* sbody = k.blk({k.cs(k.num(k.i32, "1")), k.asg(r, k.num(k.i32, "1")), k.go(L), k.dflt(), k.asg(r, k.num(k.i32, "2"))});
    const Stmt* c = twin ? k.chk(k.eq_c(k.v(r), k.i32, "1")) : k.chk(k.range(k.v(r), k.i32, "1", "12"));
    f->body = k.blk({k.decl(r, k.num(k.i32, "0")), k.sw(k.pv(f, 0), sbody, k.jid()), k.asg(r, k.add(k.v(r), k.num(k.i32, "10"))),
                     k.lab(L), c, k.ret(k.v(r))});
    k.done(f);
    if (twin) k.fail("check", c->loc);
  };
  lc("goto_out_of_switch", "goto out of a switch case to a label after the switch's fallout code", [=](Kit& k) { from_switch(k, false); });
  lc("goto_out_of_switch_twin", "the same: r == 1 does not hold on the default path", [=](Kit& k) { from_switch(k, true); });

  lc("goto_unreferenced_label_ok", "a label no goto targets lowers to nothing", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    f->body = k.blk({k.lab(k.jid()), k.ret(k.pv(f, 0))});
    k.done(f);
  });
  lc("goto_over_statement_and_before_declaration", "a goto over an assignment to a label with a declaration after it", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* r = k.local(f, "r", k.i32);
    Var* y = k.local(f, "y", k.i32);
    const int L = k.jid();
    f->body = k.blk({k.decl(r, k.num(k.i32, "0")), k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "5")), k.blk({k.go(L)})), k.asg(r, k.num(k.i32, "1")),
                     k.lab(L), k.decl(y, k.add(k.v(r), k.num(k.i32, "1"))), k.chk(k.range(k.v(y), k.i32, "1", "2")), k.ret(k.v(y))});
    k.done(f);
  });

  // ---- functions that do not return ----
  lc("noreturn_calls_abort_on_every_path", "a noreturn function whose every path calls a bodyless noreturn function verifies", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = fn_n(k);
    f->noreturn = true;
    f->body = k.blk({k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({k.es(k.call(a, {}))})), k.es(k.call(a, {}))});
    k.done(f);
  });
  lc("noreturn_end_reachable", "a noreturn function that can reach its closing brace", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = fn_n(k);
    f->noreturn = true;
    f->body = k.blk({k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({k.es(k.call(a, {}))}))});
    k.done(f);
    k.fail("noreturn", f->end_loc);
  });
  lc("noreturn_return_reachable", "a noreturn function with a return that can be reached", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = fn_n(k);
    f->noreturn = true;
    const Stmt* r = k.ret(nullptr);
    f->body = k.blk({k.iff(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.blk({r})), k.es(k.call(a, {}))});
    k.done(f);
    k.fail("noreturn", r->loc);
  });
  lc("noreturn_value_return_reachable", "a non-void noreturn function: the reachable return is a noreturn error, not a missing-return", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    f->noreturn = true;
    const Stmt* r = k.ret(k.num(k.i32, "1"));
    f->body = k.blk({r});
    k.done(f);
    k.fail("noreturn", r->loc);
  });
  lc("noreturn_value_end_reachable", "a non-void noreturn function that falls off its end: noreturn at the brace", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    f->noreturn = true;
    f->body = k.blk({});
    k.done(f);
    k.fail("noreturn", f->end_loc);
  });
  lc("noreturn_unreachable_return_ok", "a return after a call that does not return is unreachable: no error", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = fn_n(k);
    f->noreturn = true;
    f->body = k.blk({k.es(k.call(a, {})), k.ret(nullptr)});
    k.done(f);
  });
  lc("noreturn_call_makes_rest_unreachable", "after abort() in a measured loop, check(false) holds", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    const Stmt* w = k.loop(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 0)))}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))},
                           k.blk({k.iff(k.eq_c(k.v(i), k.i32, "50"), k.blk({k.es(k.call(a, {})), k.chk(k.no())})), plus1(k, i)}), nullptr, LF::While, 0);
    f->body = k.blk({k.decl(i, k.num(k.i32, "0")), w, k.ret(k.v(i))});
    k.done(f);
  });
  lc("noreturn_without_call_check_false_fails", "the same check(false) without the call is reachable", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    const Stmt* c = k.chk(k.no());
    f->body = k.blk({k.iff(k.eq_c(k.pv(f, 0), k.i32, "50"), k.blk({c})), k.ret(k.pv(f, 0))});
    k.done(f);
    k.fail("check", c->loc);
  });

  // ---- assert ----
  auto div_assert = [=](Kit& k, int variant) {  // 0 ok, 1 asserted fact false, 2 divisor may be 0
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.req(f, k.range(k.F(f, 1), k.i32, variant == 2 ? "0" : "1", "100"));
    const Expr* d = k.div(k.pv(f, 0), k.pv(f, 1));
    const Stmt* a = k.asrt(k.ge_c(d, k.i32, variant == 1 ? "1" : "0"));
    f->body = k.blk({a});
    k.done(f);
    if (variant == 1) k.fail("check", a->loc, 1, "", "assert");
    if (variant == 2) k.fail("division-by-zero", d->loc);
  };
  lc("assert_with_division_ok", "assert(a / b >= 0) with b >= 1", [=](Kit& k) { div_assert(k, 0); });
  lc("assert_fails_as_check", "assert(a / b >= 1) may not hold: a `check` error that names assert", [=](Kit& k) { div_assert(k, 1); });
  lc("assert_obligation_first", "assert(a / b >= 0) with b possibly 0: the division is reported", [=](Kit& k) { div_assert(k, 2); });
  auto call_assert = [=](Kit& k, bool good) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    k.req(g, k.range(k.F(g, 0), k.i32, "0", "100"));
    if (good) k.ens(g, k.gt(k.R(g), k.num(k.i32, "0")));
    k.trusted(g);
    Function* f = k.fn("f", k.vd, {{"x", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    const Stmt* a = k.asrt(k.gt(k.call(g, {k.pv(f, 0)}), k.num(k.i32, "0")));
    f->body = k.blk({a});
    k.done(f);
    if (!good) k.fail("check", a->loc, 1, "", "assert");
  };
  lc("assert_with_call_ok", "assert(g(x) > 0) where g ensures a positive result: the call is hoisted like any other", [=](Kit& k) { call_assert(k, true); });
  lc("assert_with_call_fails", "assert(g(x) > 0) where g has no such ensures", [=](Kit& k) { call_assert(k, false); });
  lc("assert_false_reachable_fails", "assert(0) on a reachable path", [=](Kit& k) {
    Function* f = fn_n(k);
    const Stmt* a = k.asrt(k.no());
    f->body = k.blk({a});
    k.done(f);
    k.fail("check", a->loc, 1, "", "assert");
  });
  lc("assert_after_abort_is_vacuous", "after a call that does not return, assert(0) holds", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = fn_n(k);
    f->body = k.blk({k.es(k.call(a, {})), k.asrt(k.no())});
    k.done(f);
  });
}

void define_goto_rejections() {
  using LF = LoopForm;
  auto plain = [](Kit& k, std::vector<const Stmt*> body) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk(std::move(body));
    k.done(f);
    return f;
  };
  rc("goto_backward", "goes backward", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.lab(L), k.go(L)});
  });
  rc("goto_enters_block", "enters a block", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.go(L), k.blk({k.lab(L)})});
  });
  rc("goto_enters_if_branch", "enters a block", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.go(L), k.iff(k.yes(), k.blk({k.lab(L)}))});
  });
  rc("goto_enters_loop", "enters a block", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.go(L), k.whl(k.yes(), {}, {k.cl(k.num(k.i32, "0"))}, k.blk({k.lab(L)}))});
  });
  rc("goto_enters_switch_body", "enters a block", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.go(L), k.sw(k.num(k.i32, "1"), k.blk({k.cs(k.num(k.i32, "1")), k.lab(L)}), k.jid())});
  });
  rc("goto_enters_do_once", "enters a block", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.go(L), k.loop(k.no(), {}, {}, k.blk({k.lab(L)}), nullptr, LF::DoOnce, k.jid())});
  });
  rc("goto_skips_declaration", "skips a declaration", [=](Kit& k) {
    const int L = k.jid();
    Function* f = k.fn("f", k.vd);
    Var* x = k.local(f, "x", k.i32);
    f->body = k.blk({k.go(L), k.decl(x, k.num(k.i32, "1")), k.lab(L)});
    k.done(f);
  });
  rc("goto_missing_label", "not in the function", [=](Kit& k) { plain(k, {k.go(k.jid())}); });
  rc("label_id_used_twice", "used twice", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.lab(L), k.lab(L)});
  });
  rc("label_id_of_a_loop", "and a label", [=](Kit& k) {
    const int L = k.jid();
    plain(k, {k.lab(L), k.loop(k.no(), {}, {}, k.blk({}), nullptr, LF::DoOnce, L)});
  });
  rc("label_not_directly_in_a_block", "not directly in a block", [=](Kit& k) { plain(k, {k.iff(k.yes(), k.lab(k.jid()))}); });
  rc("noreturn_with_ensures", "declared not to return", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = k.fn("f", k.vd, {{"x", k.i32}});
    f->noreturn = true;
    k.ens(f, k.gt(k.F(f, 0), k.num(k.i32, "0")));
    f->body = k.blk({k.es(k.call(a, {}))});
    k.done(f);
  });
}

// Named constants and case values (decision 0017): the prologue of every method with a body.
void define_named_constants() {
  lc("nc_initializer_shift_fails", "const int A = 1 << 40; (EDG folds it to 0): the shift is an error at the initializer", [](Kit& k) {
    const Expr* init = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.nc("A", k.i32, init, "0");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("shift", init->loc, 1, "", "shift count");  // one method, no contract: no __wf
  });
  lc("nc_guard_holds_and_has_a_map_row", "const int B = 2 + 3; with the front end's value 5: obligations hold, the internal guard holds", [](Kit& k) {
    k.nc("B", k.i32, k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), "5");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "the value of `B` as written differs from the front end's");
  });
  lc("nc_guard_fails", "const int C = 2 + 3; with a front-end value 6: the internal guard fails at the declaration", [](Kit& k) {
    NamedConst* c = k.nc("C", k.i32, k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), "6");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "the value of `C` as written");
  });
  lc("nc_used_in_contract_and_body", "const int N = 20 + 20 (value 40) read as the literal 40 in a requires, the body and an ensures", [](Kit& k) {
    k.nc("N", k.i32, k.add(k.num(k.i32, "20"), k.num(k.i32, "20")), "40");
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "39"));
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), k.num(k.i32, "40"))));
    f->body = k.blk({k.chk(k.lt(k.pv(f, 0), k.num(k.i32, "40"))), k.ret(k.add(k.pv(f, 0), k.num(k.i32, "40")))});
    k.done(f);
  });
  lc("nc_prologue_in_the_wf_method", "a failing constant initializer is reported in a function's method and in its __wf method", [](Kit& k) {
    const Expr* init = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.nc("A", k.i32, init, "0");
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    f->body = k.blk({k.ret(k.pv(f, 0))});
    k.done(f);
    k.fail("shift", init->loc, 2, "", "shift count");
  });
  lc("nc_bool", "const bool T = !false (value 1) and const bool U = true: the guard compares Bool values", [](Kit& k) {
    k.nc("T", k.bl, k.lnot(k.no()), "1");
    k.nc("U", k.bl, k.yes(), "1");
    Function* f = k.fn("f", k.bl);
    f->body = k.blk({k.ret(k.yes())});
    k.done(f);
    k.require_map("internal", "the value of `T` as written differs from the front end's");
  });
  lc("nc_bool_value_differs", "const bool V = true with the front end's value 0: a bare literal that differs still gets its guard, which fails", [](Kit& k) {
    NamedConst* c = k.nc("V", k.bl, k.yes(), "0");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "the value of `V` as written");
  });
  lc("case_value_checked_on_every_path", "if (false) switch (x) { case 1 << 40: ... }: the case value's shift is checked at entry, path-independently", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* bad = k.cs(k.shl(k.num(k.i32, "1"), k.num(k.i32, "40")), "0");
    const Stmt* swt = k.sw(k.pv(f, 0), k.blk({bad, k.ret(k.num(k.i32, "1"))}), id);
    f->body = k.blk({k.iff(k.no(), swt), k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail("shift", bad->loc, 1, "", "shift count");
  });
  lc("case_guard_holds_and_has_a_map_row", "case 1 << 3 with the front end's value 8: the internal guard holds; a plain `case 3` has no guard", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* body = k.blk({k.cs(k.shl(k.num(k.i32, "1"), k.num(k.i32, "3")), "8"), k.ret(k.num(k.i32, "1")), k.cs(k.num(k.i32, "3")), k.ret(k.num(k.i32, "2")),
                              k.dflt(), k.ret(k.num(k.i32, "0"))});
    f->body = k.blk({k.sw(k.pv(f, 0), body, id)});
    k.done(f);
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "8"), k.eq_c(k.R(f), k.i32, "1")));
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "3"), k.eq_c(k.R(f), k.i32, "2")));
    k.require_map("internal", "case value as written differs from the front end's");
  });
  lc("case_guard_fails", "case 1 + 1 with a front-end value 3: the internal guard fails at the case label", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* bad = k.cs(k.add(k.num(k.i32, "1"), k.num(k.i32, "1")), "3");
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({bad, k.ret(k.num(k.i32, "1"))}), id), k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail("internal", bad->loc, 1, "", "case value as written");
  });
  lc("case_dispatch_uses_the_front_end_value", "case N + 1 (N = 8, so 9): the dispatch compares with 9, so x == 9 returns 1", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* swt = k.sw(k.pv(f, 0), k.blk({k.cs(k.add(k.num(k.i32, "8"), k.num(k.i32, "1")), "9"), k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))}), id);
    f->body = k.blk({swt});
    k.done(f);
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "9"), k.eq_c(k.R(f), k.i32, "1")));
    k.ens(f, k.implies(k.ne(k.F(f, 0), k.num(k.i32, "9")), k.eq_c(k.R(f), k.i32, "0")));
  });
  lc("case_local_const_in_a_case_value", "const int k = 2 + 3 (a local constant of f) named by `case k`: its initializer is checked in f's prologue", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.lnc(f, "k", k.i32, k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), "5");
    const int id = k.jid();
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "5")), k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))}), id)});
    k.done(f);
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "5"), k.eq_c(k.R(f), k.i32, "1")));
    k.require_map("internal", "the value of `k` as written differs from the front end's");
  });
  lc("case_local_const_shift_fails", "const int ks = 1 << 40; named by `case ks` (folded 0): the local constant's shift is an error in f's method", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const Expr* init = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.lnc(f, "ks", k.i32, init, "0");
    const int id = k.jid();
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "0")), k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))}), id)});
    k.done(f);
    k.fail("shift", init->loc, 1, "", "shift count");
  });
  lc("case_value_checked_when_requires_is_unsatisfiable", "requires x > 0 && x < 0 makes the method vacuous; the case value's shift is still reported, by the __wf method", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.land(k.gt(k.F(f, 0), k.num(k.i32, "0")), k.lt(k.F(f, 0), k.num(k.i32, "0"))));
    const int id = k.jid();
    const Stmt* bad = k.cs(k.shl(k.num(k.i32, "1"), k.num(k.i32, "40")), "0");
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({bad, k.ret(k.num(k.i32, "1"))}), id), k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail_only("shift", bad->loc, 2, 1, "shift count");  // the method's own copy is vacuous; the __wf copy fails
  });
  lc("nc_local_const_checked_in_the_wf_method", "a local constant with a failing shift is reported in the function's method and in its __wf method", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const Expr* init = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.lnc(f, "ks", k.i32, init, "0");
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    const int id = k.jid();
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "0")), k.ret(k.num(k.i32, "1"))}), id), k.ret(k.num(k.i32, "0"))});
    k.done(f);
    k.fail("shift", init->loc, 2, "", "shift count");
  });
  lc("case_nested_switch_values_in_source_order", "a nested switch's case value is checked in the prologue too (its shift fails)", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int outer = k.jid();
    const int inner = k.jid();
    const Stmt* bad = k.cs(k.shl(k.num(k.i32, "1"), k.num(k.i32, "40")), "0");
    const Stmt* isw = k.sw(k.pv(f, 0), k.blk({bad, k.ret(k.num(k.i32, "1"))}), inner);
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "7")), isw, k.dflt(), k.ret(k.num(k.i32, "0"))}), outer)});
    k.done(f);
    k.fail("shift", bad->loc, 1, "", "shift count");
  });
}

void define_named_constant_rejections() {
  rc("nc_initializer_mentions_a_variable", "mentions a variable", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.nc("A", k.i32, k.add(k.pv(f, 0), k.num(k.i32, "1")), "1");
    f->body = k.blk({});
    k.done(f);
  });
  rc("nc_initializer_has_another_type", "has type", [](Kit& k) {
    k.nc("A", k.i64, k.num(k.i32, "1"), "1");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
  });
  rc("nc_value_out_of_range", "is not a value of", [](Kit& k) {
    k.nc("A", k.i32, k.num(k.i32, "1"), "2147483648");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
  });
  rc("case_folded_out_of_range", "is not a value of", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "1"), "4294967296")}), k.jid())});
    k.done(f);
  });
  rc("case_without_a_front_end_value", "a case without its front-end value", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.b.case_label(k.num(k.i32, "1"), "", k.at())}), k.jid())});
    k.done(f);
  });
  rc("case_duplicate_front_end_value", "appears twice", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.add(k.num(k.i32, "1"), k.num(k.i32, "1")), "2"), k.cs(k.num(k.i32, "2"))}), k.jid())});
    k.done(f);
  });
}

void define_calls_loops_and_more() {
  define_named_constants();
  define_named_constant_rejections();
  define_calls();
  define_loops();
  define_control_flow();
  define_termination();
  define_returns_and_contracts();
  define_wellformedness();
  define_compound_assignment();
  define_misc();
  define_goto_noreturn_assert();
  define_goto_rejections();
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
