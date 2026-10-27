// (included by selftest.cpp) Calls in expressions (decision 0021): calls in guarded operands, in loop conditions and next to other
// calls, the dependency-chain rule and its rejections, the explicit termination check, and the self-test hook of the order check.
// Programs without these shapes are not touched by anything here.
namespace {

void define_calls_in_expressions() {
  using LF = LoopForm;
  auto I = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  // A trusted `int g(int x)` with `ensures r == x` (and, when `positive`, `requires x > 0`).
  auto g_id = [](Kit& k, bool positive = false) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    if (positive) k.req(g, k.gt(k.F(g, 0), k.num(k.i32, "0")));
    k.ens(g, k.eq(k.R(g), k.F(g, 0)));
    k.trusted(g);
    return g;
  };
  // A trusted `int never(int x)` (`ensures false`: it does not return), `int zero(int y)` (any result) and `int h0()`.
  auto never = [](Kit& k) {
    Function* n = k.fn("never", k.i32, {{"x", k.i32}});
    k.ens(n, k.no());
    k.trusted(n);
    return n;
  };
  auto zero = [](Kit& k) {
    Function* z = k.fn("zero", k.i32, {{"y", k.i32}});
    k.trusted(z);
    return z;
  };
  auto h0 = [](Kit& k) {
    Function* h = k.fn("h0", k.i32);
    k.trusted(h);
    return h;
  };
  // A trusted `int name(int x)` without a contract.
  auto t1 = [](Kit& k, const char* name) {
    Function* t = k.fn(name, k.i32, {{"x", k.i32}});
    k.trusted(t);
    return t;
  };
  // A case and its twin: `build(k, wrong)` returns the `return` statement the postcondition is asserted at.
  auto twin = [](const char* name, const char* doc, std::function<const Stmt*(Kit&, bool)> build) {
    lc(name, doc, [build](Kit& k) { build(k, false); });
    lc(std::string(name) + "_wrong", std::string(doc) + " (with a wrong postcondition)", [build](Kit& k) {
      const Stmt* r = build(k, true);
      k.fail("postcondition", r->loc);
    });
  };

  // ---- calls next to calls (the shapes 0012 item 6 refused) ----
  twin("calls_sum_of_two", "g(1) + g(2) with g(x) == x: the unsequenced calls are ordered, the sum is 3", [=](Kit& k, bool wrong) {
    Function* g = g_id(k);
    Function* f = k.fn("f", k.i32);
    k.ens(f, k.eq(k.R(f), I(k, wrong ? "4" : "3")));
    const Stmt* r = k.ret(k.add(k.call(g, {I(k, "1")}), k.call(g, {I(k, "2")})));
    f->body = k.blk({r});
    k.done(f);
    return r;
  });
  twin("calls_sibling_arguments", "h(g(1), g(2)): two calls in different arguments, int parameters: no check depends on only one of them",
       [=](Kit& k, bool wrong) {
         Function* g = g_id(k);
         Function* h = k.fn("h", k.i32, {{"x", k.i32}, {"y", k.i32}});
         k.ens(h, k.eq(k.R(h), k.F(h, 0)));
         k.trusted(h);
         Function* f = k.fn("f", k.i32);
         k.ens(f, k.eq(k.R(f), I(k, wrong ? "2" : "1")));
         const Stmt* r = k.ret(k.call(h, {k.call(g, {I(k, "1")}), k.call(g, {I(k, "2")})}));
         f->body = k.blk({r});
         k.done(f);
         return r;
       });
  lc("calls_sibling_arguments_precondition", "h(g(1), g(2)) with requires x < y: one clause over both arguments depends on both calls",
     [=](Kit& k) {
       Function* g = g_id(k);
       Function* h = k.fn("h", k.i32, {{"x", k.i32}, {"y", k.i32}});
       k.req(h, k.lt(k.F(h, 0), k.F(h, 1)));
       k.trusted(h);
       Function* f = k.fn("f", k.i32);
       f->body = k.blk({k.ret(k.call(h, {k.call(g, {I(k, "1")}), k.call(g, {I(k, "2")})}))});
       k.done(f);
     });
  lc("calls_sibling_arguments_precondition_fails", "h(g(2), g(1)) with requires x < y: the precondition does not hold", [=](Kit& k) {
    Function* g = g_id(k);
    Function* h = k.fn("h", k.i32, {{"x", k.i32}, {"y", k.i32}});
    k.req(h, k.lt(k.F(h, 0), k.F(h, 1)));
    k.trusted(h);
    Function* f = k.fn("f", k.i32);
    const Expr* c = k.call(h, {k.call(g, {I(k, "2")}), k.call(g, {I(k, "1")})});
    f->body = k.blk({k.ret(c)});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  twin("calls_comma_in_a_sum", "(g(1), 0) + g(2): the comma orders its own call only, the sum is 2", [=](Kit& k, bool wrong) {
    Function* g = g_id(k);
    Function* f = k.fn("f", k.i32);
    k.ens(f, k.eq(k.R(f), I(k, wrong ? "3" : "2")));
    const Stmt* r = k.ret(k.add(k.comma(k.call(g, {I(k, "1")}), I(k, "0")), k.call(g, {I(k, "2")})));
    f->body = k.blk({r});
    k.done(f);
    return r;
  });
  lc("calls_sum_overflow", "g() + g() with an unconstrained int result: the sum may overflow", [=](Kit& k) {
    Function* g = t1(k, "g");
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    const Expr* s = k.add(k.call(g, {I(k, "1")}), k.call(g, {I(k, "2")}));
    f->body = k.blk({k.ret(s)});
    k.done(f);
    k.fail("overflow", s->loc);
  });

  // ---- the order: a check that does not depend on a call that does not return is not hidden by it ----
  lc("calls_pin_never_then_division_by_zero", "never(x) + 100 / zero(y): zero's call and the divisor check come first, so the division by zero is found",
     [=](Kit& k) {
       Function* n = never(k);
       Function* z = zero(k);
       Function* f = k.fn("f", k.i32, {{"x", k.i32}, {"y", k.i32}});
       const Expr* d = k.div(I(k, "100"), k.call(z, {k.pv(f, 1)}));
       f->body = k.blk({k.ret(k.add(k.call(n, {k.pv(f, 0)}), d))});
       k.done(f);
       k.fail("division-by-zero", d->loc);
     });
  lc("calls_pin_division_by_zero_after_never_in_source_order", "100 / zero(y) + never(x): the same program with the operands swapped", [=](Kit& k) {
    Function* n = never(k);
    Function* z = zero(k);
    Function* f = k.fn("f", k.i32, {{"x", k.i32}, {"y", k.i32}});
    const Expr* d = k.div(I(k, "100"), k.call(z, {k.pv(f, 1)}));
    f->body = k.blk({k.ret(k.add(d, k.call(n, {k.pv(f, 0)})))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("calls_reorder_chain_ok", "f(g(x)) + 100 / h(y): emitted h, its divisor check, g, f; every result is in a range that cannot overflow", [=](Kit& k) {
    Function* g = t1(k, "g");
    Function* f1 = k.fn("f1", k.i32, {{"x", k.i32}});
    k.ens(f1, k.range(k.R(f1), k.i32, "0", "1000"));
    k.trusted(f1);
    Function* h = k.fn("h", k.i32, {{"y", k.i32}});
    k.ens(h, k.range(k.R(h), k.i32, "1", "100"));
    k.trusted(h);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}});
    c->body = k.blk({k.ret(k.add(k.call(f1, {k.call(g, {k.pv(c, 0)})}), k.div(I(k, "100"), k.call(h, {k.pv(c, 1)}))))});
    k.done(c);
  });
  lc("calls_reorder_chain_division_by_zero", "f(never(x)) + 100 / zero(y): emitted zero, the check (which fails), never, f", [=](Kit& k) {
    Function* n = never(k);
    Function* f1 = t1(k, "f1");
    Function* z = zero(k);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}});
    const Expr* d = k.div(I(k, "100"), k.call(z, {k.pv(c, 1)}));
    c->body = k.blk({k.ret(k.add(k.call(f1, {k.call(n, {k.pv(c, 0)})}), d))});
    k.done(c);
    k.fail("division-by-zero", d->loc);
  });
  lc("calls_reorder_dependent_check_follows", "never(x) + 1 / ...: a check that depends on the call that does not return follows it and is unreachable", [=](Kit& k) {
    Function* n = never(k);
    Function* z = zero(k);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}});
    c->body = k.blk({k.ret(k.add(k.call(n, {k.pv(c, 0)}), k.call(z, {k.pv(c, 1)})))});
    k.done(c);
  });

  // ---- a branch with a call comes right after what its condition depends on ----
  lc("calls_branch_before_never_division_by_zero", "never(k) + (a > 0 && 100 / zero(a) > 0): the branch with the call is emitted first, its divisor check is found",
     [=](Kit& k) {
       Function* n = never(k);
       Function* z = zero(k);
       Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"k", k.i32}});
       const Expr* d = k.div(I(k, "100"), k.call(z, {k.pv(c, 0)}));
       c->body = k.blk({k.ret(k.add(k.call(n, {k.pv(c, 1)}), k.bi(k.i32, k.land(k.gt(k.pv(c, 0), I(k, "0")), k.gt(d, I(k, "0"))))))});
       k.done(c);
       k.fail("division-by-zero", d->loc);
     });
  lc("calls_branch_before_never_ok", "never(k) + (a > 0 && 100 / pos(a) > 0): the callee's result is positive, no check fails", [=](Kit& k) {
    Function* n = never(k);
    Function* p = k.fn("pos", k.i32, {{"x", k.i32}});
    k.ens(p, k.range(k.R(p), k.i32, "1", "100"));
    k.trusted(p);
    Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"k", k.i32}});
    c->body = k.blk({k.ret(k.add(k.call(n, {k.pv(c, 1)}), k.bi(k.i32, k.land(k.gt(k.pv(c, 0), I(k, "0")), k.gt(k.div(I(k, "100"), k.call(p, {k.pv(c, 0)})), I(k, "0"))))))});
    k.done(c);
  });
  lc("calls_branch_after_call_in_source_order", "(a > 0 && 100 / zero(a) > 0) + never(k): the same with the branch first in the source", [=](Kit& k) {
    Function* n = never(k);
    Function* z = zero(k);
    Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"k", k.i32}});
    const Expr* d = k.div(I(k, "100"), k.call(z, {k.pv(c, 0)}));
    c->body = k.blk({k.ret(k.add(k.bi(k.i32, k.land(k.gt(k.pv(c, 0), I(k, "0")), k.gt(d, I(k, "0")))), k.call(n, {k.pv(c, 1)})))});
    k.done(c);
    k.fail("division-by-zero", d->loc);
  });

  // ---- calls in guarded operands ----
  auto pos = [=](Kit& k) { return g_id(k, true); };
  twin("calls_guarded_and", "a > 0 && pos(a) > 1: the call runs, with its precondition checked, only when a > 0", [=](Kit& k, bool wrong) {
    Function* p = pos(k);
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    k.ens(f, k.eq(k.R(f), k.gt(k.F(f, 0), I(k, wrong ? "2" : "1"))));
    const Stmt* r = k.ret(k.land(k.gt(k.pv(f, 0), I(k, "0")), k.gt(k.call(p, {k.pv(f, 0)}), I(k, "1"))));
    f->body = k.blk({r});
    k.done(f);
    return r;
  });
  lc("calls_guarded_and_precondition", "a >= 0 && pos(a) > 1: a == 0 reaches the call", [=](Kit& k) {
    Function* p = pos(k);
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    const Expr* c = k.call(p, {k.pv(f, 0)});
    f->body = k.blk({k.ret(k.land(k.ge(k.pv(f, 0), I(k, "0")), k.gt(c, I(k, "1"))))});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  twin("calls_guarded_or", "a <= 0 || pos(a) > 1: the call runs only when a > 0", [=](Kit& k, bool wrong) {
    Function* p = pos(k);
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    k.ens(f, k.eq(k.R(f), k.lor(k.le(k.F(f, 0), I(k, "0")), k.gt(k.F(f, 0), I(k, wrong ? "2" : "1")))));
    const Stmt* r = k.ret(k.lor(k.le(k.pv(f, 0), I(k, "0")), k.gt(k.call(p, {k.pv(f, 0)}), I(k, "1"))));
    f->body = k.blk({r});
    k.done(f);
    return r;
  });
  lc("calls_guarded_or_precondition", "a < 0 || pos(a) > 1: a == 0 reaches the call", [=](Kit& k) {
    Function* p = pos(k);
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    const Expr* c = k.call(p, {k.pv(f, 0)});
    f->body = k.blk({k.ret(k.lor(k.lt(k.pv(f, 0), I(k, "0")), k.gt(c, I(k, "1"))))});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  twin("calls_cond_arm", "a > 0 ? pos(a) : 0: the call is in an arm of ?:", [=](Kit& k, bool wrong) {
    Function* p = pos(k);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "0")), k.F(f, 0), I(k, wrong ? "1" : "0"))));
    const Stmt* r = k.ret(k.cond(k.gt(k.pv(f, 0), I(k, "0")), k.call(p, {k.pv(f, 0)}), I(k, "0")));
    f->body = k.blk({r});
    k.done(f);
    return r;
  });
  lc("calls_cond_arm_precondition", "a >= 0 ? pos(a) : 0: a == 0 reaches the call", [=](Kit& k) {
    Function* p = pos(k);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    const Expr* c = k.call(p, {k.pv(f, 0)});
    f->body = k.blk({k.ret(k.cond(k.ge(k.pv(f, 0), I(k, "0")), c, I(k, "0")))});
    k.done(f);
    k.fail("precondition", c->loc);
  });
  twin("calls_effect_in_guarded_operand", "int x = 0; b && g(x++) > 0: the write is in the branch, x is 1 exactly when b", [=](Kit& k, bool wrong) {
    Function* g = g_id(k);
    Function* f = k.fn("f", k.i32, {{"b", k.bl}});
    Var* x = k.local(f, "x", k.i32);
    const Stmt* r = k.ret(k.v(x));
    k.ens(f, k.eq(k.R(f), k.cond(k.F(f, 0), I(k, wrong ? "2" : "1"), I(k, "0"))));
    f->body = k.blk({k.decl(x, I(k, "0")), k.es(k.bi(k.i32, k.land(k.pv(f, 0), k.gt(k.call(g, {k.incE(x)}), I(k, "0"))))), r});
    k.done(f);
    return r;
  });

  // ---- calls in loop conditions ----
  // A trusted `int next(int q0, int q1)` with `ensures r == q0 + 1` and `requires q0 < q1` (loose) or `q0 + 1 < q1` (tight).
  auto next = [=](Kit& k, bool tight) {
    Function* nx = k.fn("next", k.i32, {{"q0", k.i32}, {"q1", k.i32}});
    k.req(nx, k.range(k.F(nx, 0), k.i32, "0", "1000"));
    k.req(nx, k.range(k.F(nx, 1), k.i32, "0", "1000"));
    k.req(nx, tight ? k.lt(k.add(k.F(nx, 0), I(k, "1")), k.F(nx, 1)) : k.lt(k.F(nx, 0), k.F(nx, 1)));
    k.ens(nx, k.eq(k.R(nx), k.add(k.F(nx, 0), I(k, "1"))));
    k.trusted(nx);
    return nx;
  };
  // while (next(i, n) < n) { i = i + 1; }  return i;   n in [1, 100], invariant 0 <= i <= n - 1, measure n - i, ensures r == n - 1.
  // variant: 0 right; 1 a tight callee (its precondition fails at the last evaluation); 2 a wrong postcondition; 3 a body that
  // does not move i (the measure does not decrease); 4 an invariant that is false on entry
  auto top_test = [=](Kit& k, int variant) {
    Function* nx = next(k, variant == 1);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    k.ens(f, k.eq(k.R(f), variant == 2 ? k.F(f, 0) : k.sub(k.F(f, 0), I(k, "1"))));
    Var* i = k.local(f, "i", k.i32);
    const Expr* c = k.call(nx, {k.v(i), k.pv(f, 0)});
    Clause inv = k.cl(variant == 4 ? k.range(k.v(i), k.i32, "1", "100") : k.land(k.ge(k.v(i), I(k, "0")), k.le(k.v(i), k.sub(k.pv(f, 0), I(k, "1")))));
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    const Stmt* body = variant == 3 ? k.asg(i, k.add(k.v(i), I(k, "0"))) : k.asg(i, k.add(k.v(i), I(k, "1")));
    const Stmt* w = k.loop(k.lt(c, k.pv(f, 0)), {inv}, {m}, body, nullptr, LF::While, k.jid());
    const Stmt* r = k.ret(k.v(i));
    f->body = k.blk({k.decl(i, I(k, "0")), w, r});
    k.done(f);
    if (variant == 1) k.fail_only("precondition", c->loc, 3, 2);  // the third clause of next: q0 + 1 < q1
    if (variant == 2) k.fail("postcondition", r->loc);
    if (variant == 3) k.fail("termination", m.loc);
    if (variant == 4) k.fail("invariant-entry", inv.loc);
  };
  lc("calls_loop_condition_ok", "while (next(i, n) < n) i = i + 1: the call runs at every evaluation, its ensures is known after it", [=](Kit& k) { top_test(k, 0); });
  lc("calls_loop_condition_precondition_last_evaluation", "the callee requires q0 + 1 < q1: it holds until the last evaluation, where i == n - 1",
     [=](Kit& k) { top_test(k, 1); });
  lc("calls_loop_condition_wrong_postcondition", "the same loop with the postcondition r == n", [=](Kit& k) { top_test(k, 2); });
  lc("calls_loop_condition_measure", "the body does not move i: the measure of the loop does not decrease", [=](Kit& k) { top_test(k, 3); });
  lc("calls_loop_condition_invariant_entry", "the invariant 1 <= i <= 100 is false on entry", [=](Kit& k) { top_test(k, 4); });
  // do { i = i + 1; } while (next(i, n) < n);  n in [2, 100], invariant 0 <= i <= n - 2 at the start of every iteration.
  auto do_loop = [=](Kit& k, int variant) {
    Function* nx = next(k, variant == 1);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "2", "100"));
    k.ens(f, k.eq(k.R(f), variant == 2 ? k.F(f, 0) : k.sub(k.F(f, 0), I(k, "1"))));
    Var* i = k.local(f, "i", k.i32);
    const Expr* c = k.call(nx, {k.v(i), k.pv(f, 0)});
    Clause inv = k.cl(k.land(k.ge(k.v(i), I(k, "0")), k.le(k.v(i), k.sub(k.pv(f, 0), I(k, "2")))));
    Clause m = k.cl(k.gap(k.pv(f, 0), k.v(i)));
    const Stmt* body = variant == 3 ? k.asg(i, k.add(k.v(i), I(k, "0"))) : k.asg(i, k.add(k.v(i), I(k, "1")));
    const Stmt* w = k.loop(k.lt(c, k.pv(f, 0)), {inv}, {m}, body, nullptr, LF::DoWhile, k.jid());
    const Stmt* r = k.ret(k.v(i));
    f->body = k.blk({k.decl(i, I(k, "0")), w, r});
    k.done(f);
    if (variant == 1) k.fail_only("precondition", c->loc, 3, 2);  // the third clause of next: q0 + 1 < q1
    if (variant == 2) k.fail("postcondition", r->loc);
    if (variant == 3) k.fail("termination", m.loc);
  };
  lc("calls_do_condition_ok", "do i = i + 1; while (next(i, n) < n): the call is at the end of the body", [=](Kit& k) { do_loop(k, 0); });
  lc("calls_do_condition_precondition_last_evaluation", "the tight callee: its precondition fails at the last evaluation", [=](Kit& k) { do_loop(k, 1); });
  lc("calls_do_condition_wrong_postcondition", "the same loop with the postcondition r == n", [=](Kit& k) { do_loop(k, 2); });
  lc("calls_do_condition_measure", "the body does not move i: the measure does not decrease", [=](Kit& k) { do_loop(k, 3); });
  twin("calls_loop_condition_with_effect", "x = 0; while (g(x++) < 5) {}: the write inside the call's argument runs at every evaluation, x is 6 at the end",
       [=](Kit& k, bool wrong) {
         Function* g = g_id(k);
         Function* f = k.fn("f", k.i32);
         k.ens(f, k.eq(k.R(f), I(k, wrong ? "5" : "6")));
         Var* x = k.local(f, "x", k.i32);
         Clause inv = k.cl(k.range(k.v(x), k.i32, "0", "5"));
         Clause m = k.cl(k.gap(I(k, "5"), k.v(x)));
         const Stmt* w = k.loop(k.lt(k.call(g, {k.incE(x)}), I(k, "5")), {inv}, {m}, k.blk({}), nullptr, LF::While, k.jid());
         const Stmt* r = k.ret(k.v(x));
         f->body = k.blk({k.decl(x, I(k, "0")), w, r});
         k.done(f);
         return r;
       });

  // ---- the explicit termination check (decision 0021 item 4) ----
  // int g(int n) ensures false, requires 0 <= n <= 1000, decreases n: { if (n <= LIM) abort(); if (n == 1000) abort(); return CALLS; }
  // `build` makes the two calls.  A call emitted after another fixed point outside its PRE gets the explicit check.
  auto never_rec = [=](Kit& k, std::function<std::pair<const Expr*, const Expr*>(Kit&, Function*)> calls, int lim, const Expr** second) {
    Function* a = k.abort_fn();
    Function* g = k.fn("g", k.i32, {{"n", k.i32}});
    k.req(g, k.range(k.F(g, 0), k.i32, "0", "1000"));
    k.ens(g, k.no());
    k.dec(g, k.F(g, 0));
    auto [c1, c2] = calls(k, g);
    *second = c2;
    char buf[8];
    std::snprintf(buf, sizeof buf, "%d", lim);
    g->body = k.blk({k.iff(k.le(k.pv(g, 0), I(k, buf)), k.blk({k.es(k.call(a, {}))})),
                     k.iff(k.eq_c(k.pv(g, 0), k.i32, "1000"), k.blk({k.es(k.call(a, {}))})), k.ret(k.add(c1, c2))});
    k.done(g);
    return g;
  };
  lc("calls_termination_pair_fails", "g(n - 1) + g(n + 1) in a g that does not return: g(n + 1) does not decrease the measure; Silicon's own check never reaches it "
     "after g(n - 1), the explicit check does", [=](Kit& k) {
    const Expr* second = nullptr;
    never_rec(k, [](Kit& k2, Function* g) {
      auto i1 = [&](const char* v) { return k2.num(k2.i32, v); };
      return std::make_pair(k2.call(g, {k2.sub(k2.pv(g, 0), i1("1"))}), k2.call(g, {k2.add(k2.pv(g, 0), i1("1"))}));
    }, 0, &second);
    k.fail_only("termination", second->loc, 2, 0);  // the explicit assert (first row), not Silicon's own check at the call
  });
  lc("calls_termination_pair_ok", "g(n - 1) + g(n - 2) in a g that does not return: both decrease; the explicit check of the second call is written and holds",
     [=](Kit& k) {
       const Expr* second = nullptr;
       never_rec(k, [](Kit& k2, Function* g) {
         auto i1 = [&](const char* v) { return k2.num(k2.i32, v); };
         return std::make_pair(k2.call(g, {k2.sub(k2.pv(g, 0), i1("1"))}), k2.call(g, {k2.sub(k2.pv(g, 0), i1("2"))}));
       }, 1, &second);
       k.rows("termination", second->loc, 2);  // the explicit assert and Silicon's own check at the call
     });
  lc("calls_termination_first_call_has_no_explicit_check", "the first call of the pair is right after its PRE: Silicon's own check alone", [=](Kit& k) {
    const Expr* second = nullptr;
    const Expr* first = nullptr;
    never_rec(k, [&first](Kit& k2, Function* g) {
      auto i1 = [&](const char* v) { return k2.num(k2.i32, v); };
      first = k2.call(g, {k2.sub(k2.pv(g, 0), i1("1"))});
      return std::make_pair(first, k2.call(g, {k2.sub(k2.pv(g, 0), i1("2"))}));
    }, 1, &second);
    k.rows("termination", first->loc, 1);
  });
  // A recursive call inside a guarded operand: the branch is emitted right after its PRE, the call is first in it: Silicon's own check.
  auto guarded_rec = [=](Kit& k, bool decreasing) {
    Function* f = k.fn("f", k.bl, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.dec(f, k.F(f, 0));
    const Expr* c = k.call(f, {decreasing ? k.sub(k.pv(f, 0), I(k, "1")) : k.pv(f, 0)});
    f->body = k.blk({k.ret(k.land(k.gt(k.pv(f, 0), I(k, "0")), c))});
    k.done(f);
    if (!decreasing) k.fail("termination", c->loc);
  };
  lc("calls_termination_in_guarded_operand", "n > 0 && f(n - 1): the recursive call in the right operand of &&", [=](Kit& k) { guarded_rec(k, true); });
  lc("calls_termination_in_guarded_operand_fails", "n > 0 && f(n): the measure does not decrease", [=](Kit& k) { guarded_rec(k, false); });
  // Mutual recursion with measures of different lengths: f(a, b) decreases (a, b), g(c) decreases (c).  `h0() + g(...)` puts another call first.
  //   f(a, b): if (a == 0) return 0; return h0() + g(F_ARG)       g(c): if (c == 0) return 0; return h0() + f(G_ARGS)
  // The call f -> g has N = (arg) against M = (a, b): decreasing when arg < a, and not when arg == a (a shorter tuple that agrees is
  // not smaller).  The call g -> f has N = (x, y) against M = (c): decreasing when x < c, and also when x == c (a longer tuple that
  // agrees on the whole shorter one is smaller).
  auto unequal = [=](Kit& k, int variant) {
    Function* h = h0(k);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    Function* g = k.fn("g", k.i32, {{"c", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "100"));
    k.dec(f, k.F(f, 0));
    k.dec(f, k.F(f, 1));
    k.req(g, k.range(k.F(g, 0), k.i32, "0", "100"));
    k.dec(g, k.F(g, 0));
    // variant 0: f -> g(a - 1) and g -> f(c, 100): both decrease.  1: f -> g(a): not decreasing.  2: g -> f(c + 1, 0): not decreasing.
    const Expr* to_g = k.call(g, {variant == 1 ? k.pv(f, 0) : k.sub(k.pv(f, 0), I(k, "1"))});
    const Expr* to_f = variant == 2 ? k.call(f, {k.add(k.pv(g, 0), I(k, "1")), I(k, "0")}) : k.call(f, {k.pv(g, 0), I(k, "100")});
    // (a comparison of the two results: no sum that may overflow)
    f->body = k.blk({k.iff(k.eq_c(k.pv(f, 0), k.i32, "0"), k.blk({k.ret(I(k, "0"))})), k.ret(k.bi(k.i32, k.lt(k.call(h, {}), to_g)))});
    k.done(f);
    g->body = k.blk({k.iff(k.eq_c(k.pv(g, 0), k.i32, "0"), k.blk({k.ret(I(k, "0"))})),
                     k.iff(k.eq_c(k.pv(g, 0), k.i32, "100"), k.blk({k.ret(I(k, "1"))})), k.ret(k.bi(k.i32, k.lt(k.call(h, {}), to_f)))});
    k.done(g);
    // The explicit check precedes each call (h0() is first and outside its PRE); the failing one is the assert, not Silicon's own check.
    if (variant == 1) k.fail_only("termination", to_g->loc, 2, 0);
    if (variant == 2) k.fail_only("termination", to_f->loc, 2, 0);
    if (variant == 0) {
      k.rows("termination", to_g->loc, 2);
      k.rows("termination", to_f->loc, 2);
    }
  };
  lc("calls_termination_unequal_lengths_ok", "f(a, b) -> g(a - 1) and g(c) -> f(c, 100): a longer tuple that agrees on the shorter one decreases, and so does a smaller first component",
     [=](Kit& k) { unequal(k, 0); });
  lc("calls_termination_unequal_lengths_shorter_fails", "f(a, b) -> g(a): the shorter tuple (a) agrees with (a, b) and is not smaller", [=](Kit& k) { unequal(k, 1); });
  lc("calls_termination_unequal_lengths_longer_fails", "g(c) -> f(c + 1, 0): (c + 1, 0) is not smaller than (c)", [=](Kit& k) { unequal(k, 2); });

  // ---- the order hook ----
  rh("hook_perturb_access_order", "an access to a local was emitted before an earlier access", [](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.lower_options.perturb_access_order = true;
    f->body = k.blk({k.decl(x, k.num(k.i32, "0")), k.ret(k.comma(k.asgE(x, k.num(k.i32, "1")), k.v(x)))});
    k.done(f);
  });
  lc("calls_order_hook_off_ok", "(x = 1, x) without the hook: the write and the snapshot are in order", [](Kit& k) {
    Function* f = k.fn("f", k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "1"));
    f->body = k.blk({k.decl(x, k.num(k.i32, "0")), k.ret(k.comma(k.asgE(x, k.num(k.i32, "1")), k.v(x)))});
    k.done(f);
  });
  // ---- writes and snapshots outside the chain (decision 0025 item 2) ----
  // A write or a snapshot cannot fail, so it is not in F: it goes right after the member of its set that comes last in pi, and two of
  // them with different calls need no order between the calls.
  twin("calls_two_writes", "(x = g(a)) + (y = g(b)): two writes of different calls, neither depends on the other", [=](Kit& k, bool wrong) {
    Function* g = g_id(k);
    Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(c, k.range(k.F(c, 0), k.i32, "0", "100"));
    k.req(c, k.range(k.F(c, 1), k.i32, "0", "100"));
    k.ens(c, k.eq(k.R(c), k.add(k.F(c, 0), wrong ? k.add(k.F(c, 1), I(k, "1")) : k.F(c, 1))));
    Var* x = k.local(c, "x", k.i32);
    Var* y = k.local(c, "y", k.i32);
    const Stmt* r = k.ret(k.add(k.v(x), k.v(y)));
    c->body = k.blk({k.decl(x, I(k, "0")), k.decl(y, I(k, "0")),
                     k.es(k.add(k.asgE(x, k.call(g, {k.pv(c, 0)})), k.asgE(y, k.call(g, {k.pv(c, 1)})))), r});
    k.done(c);
    return r;
  });
  twin("calls_write_after_unrelated_call", "f(b) < (x = g(a)): the write depends on g only, f comes first in creation order and no check depends on both",
       [=](Kit& k, bool wrong) {
         Function* g = g_id(k);
         Function* f = t1(k, "f");
         Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"b", k.i32}});
         k.req(c, k.range(k.F(c, 0), k.i32, "0", "100"));
         k.ens(c, k.eq(k.R(c), wrong ? k.add(k.F(c, 0), I(k, "1")) : k.F(c, 0)));
         Var* x = k.local(c, "x", k.i32);
         const Stmt* r = k.ret(k.v(x));
         c->body = k.blk({k.decl(x, I(k, "0")), k.es(k.lt(k.call(f, {k.pv(c, 1)}), k.asgE(x, k.call(g, {k.pv(c, 0)})))), r});
         k.done(c);
         return r;
       });
  twin("calls_initialized_snapshots", "(x = g(a), x) + (y = g(b), y) with initialized x and y: the snapshots have no flag check, so no set joins the two calls",
       [=](Kit& k, bool wrong) {
         Function* g = g_id(k);
         Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"b", k.i32}});
         k.req(c, k.range(k.F(c, 0), k.i32, "0", "100"));
         k.req(c, k.range(k.F(c, 1), k.i32, "0", "100"));
         k.ens(c, k.eq(k.R(c), k.add(k.F(c, 0), wrong ? k.add(k.F(c, 1), I(k, "1")) : k.F(c, 1))));
         Var* x = k.local(c, "x", k.i32);
         Var* y = k.local(c, "y", k.i32);
         const Stmt* r = k.ret(k.add(k.comma(k.asgE(x, k.call(g, {k.pv(c, 0)})), k.v(x)), k.comma(k.asgE(y, k.call(g, {k.pv(c, 1)})), k.v(y))));
         c->body = k.blk({k.decl(x, I(k, "0")), k.decl(y, I(k, "0")), r});
         k.done(c);
         return r;
       });

  // ---- the flag read is an access of its local (decision 0025 item 1) ----
  // int x; int y = (x = (sink(f(a) > 0 && x > 0), 2)); return y + x;
  // A comma drops its left operand's calls, so the write has an empty set; the check of the flag read has the set {f}.  The write must
  // still follow the check: C reads x uninitialized when f(a) > 0.  Variant 0: the read can happen; 1: requires a <= 0 (it cannot);
  // 2: x is initialized (no flag).
  auto guarded_flag_read = [=](Kit& k, int variant) {
    Function* f = g_id(k);
    Function* sink = t1(k, "sink");
    Function* c = k.fn("c", k.i32, {{"a", k.i32}});
    if (variant == 1) k.req(c, k.le_c(k.F(c, 0), k.i32, "0"));
    Var* x = k.local(c, "x", k.i32);
    Var* y = k.local(c, "y", k.i32);
    const Expr* rd = k.v(x);
    const Expr* arg = k.bi(k.i32, k.land(k.gt(k.call(f, {k.pv(c, 0)}), I(k, "0")), k.gt(rd, I(k, "0"))));
    const Expr* value = k.asgE(x, k.comma(k.call(sink, {arg}), I(k, "2")));
    c->body = k.blk({k.decl(x, variant == 2 ? I(k, "0") : nullptr), k.decl(y, value), k.ret(k.add(k.v(y), k.v(x)))});
    k.done(c);
    if (variant == 0) k.fail("uninitialized", rd->loc);
  };
  lc("uninit_write_after_guarded_read_comma", "int x; y = (x = (sink(f(a) > 0 && x > 0), 2)): the write follows the check of the guarded read (finding A)",
     [=](Kit& k) { guarded_flag_read(k, 0); });
  lc("uninit_write_after_guarded_read_comma_path_precise", "the same with requires a <= 0: f(a) > 0 cannot hold, so the read does not happen",
     [=](Kit& k) { guarded_flag_read(k, 1); });
  lc("uninit_write_after_guarded_read_comma_initialized", "the same with int x = 0: no flag, nothing to check", [=](Kit& k) { guarded_flag_read(k, 2); });
  // int x; y = (((f(a) > 0 && x > 0), 1) && ((x = 1) == 1)); the write is in a branch, after the comma dropped f.
  auto guarded_flag_read_branch = [=](Kit& k, bool possible) {
    Function* f = g_id(k);
    Function* c = k.fn("c", k.i32, {{"a", k.i32}});
    if (!possible) k.req(c, k.le_c(k.F(c, 0), k.i32, "0"));
    Var* x = k.local(c, "x", k.i32);
    Var* y = k.local(c, "y", k.i32);
    const Expr* rd = k.v(x);
    const Expr* left = k.comma(k.bi(k.i32, k.land(k.gt(k.call(f, {k.pv(c, 0)}), I(k, "0")), k.gt(rd, I(k, "0")))), I(k, "1"));
    const Expr* value = k.bi(k.i32, k.land(k.tb(left), k.eq(k.asgE(x, I(k, "1")), I(k, "1"))));
    c->body = k.blk({k.decl(x, nullptr), k.decl(y, value), k.ret(k.add(k.v(y), k.v(x)))});
    k.done(c);
    if (possible) k.fail("uninitialized", rd->loc);
  };
  lc("uninit_write_in_branch_after_guarded_read", "int x; y = (((f(a) > 0 && x > 0), 1) && ((x = 1) == 1)): the branch with the write follows the check",
     [=](Kit& k) { guarded_flag_read_branch(k, true); });
  lc("uninit_write_in_branch_after_guarded_read_path_precise", "the same with requires a <= 0", [=](Kit& k) { guarded_flag_read_branch(k, false); });
  // int c(int a): int x = 0; x = g(a) as a value; return x;  The hook moves the write before g, which its set contains.
  auto write_after_call = [=](Kit& k, bool want_ok) {
    Function* g = g_id(k);
    Function* c = k.fn("c", k.i32, {{"a", k.i32}});
    if (want_ok) k.ens(c, k.eq(k.R(c), k.F(c, 0)));
    Var* x = k.local(c, "x", k.i32);
    c->body = k.blk({k.decl(x, I(k, "0")), k.es(k.asgE(x, k.call(g, {k.pv(c, 0)}))), k.ret(k.v(x))});
    k.done(c);
  };
  rh("hook_perturb_write_slot", "a write or a snapshot of a full expression is emitted before a fixed point of its set", [=](Kit& k) {
    k.lower_options.perturb_write_slot = true;
    write_after_call(k, false);
  });
  // The flag read is an access (decision 0025 item 1): without it the write of the comma case precedes the check, and the order check says so.
  // The comma's value carries its floor (decision 0026), so this program takes step 1 and the write follows the check without the flag access; the hook
  // `force_base_sets` makes every full expression take step 2 (today's sets), where the flag access is what orders them.
  rh("hook_drop_flag_access", "an access to a local was emitted before an earlier access", [=](Kit& k) {
    k.lower_options.drop_flag_access = true;
    k.lower_options.force_base_sets = true;
    guarded_flag_read(k, 0);
  });
  // The same program with `drop_flag_access` but without `force_base_sets`: step 1 orders the write after the check by the comma's floor, so there is no
  // internal error, and the program is what it verifies to (the `uninitialized` error at the read).
  lc("hook_drop_flag_access_step1", "int x; y = (x = (sink(f(a) > 0 && x > 0), 2)) with the flag access dropped but exact sets: the write still follows "
     "the check (step 1), so the `uninitialized` error at the read is found and there is no internal error", [=](Kit& k) {
    k.lower_options.drop_flag_access = true;
    guarded_flag_read(k, 0);
  });
  // A write that a later item depends on must be emitted before it: int x; h(b) < 100 / (x = g(a)).  The divisor check has the set {g}, the
  // write's; the hook moves the write to the last slot, behind the check.
  auto write_then_dependent_check = [=](Kit& k, bool want_ok) {
    Function* g = g_id(k);
    Function* h = t1(k, "h");
    Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(c, k.range(k.F(c, 0), k.i32, "1", "100"));
    if (want_ok) k.ens(c, k.eq(k.R(c), k.F(c, 0)));
    Var* x = k.local(c, "x", k.i32);
    c->body = k.blk({k.decl(x, I(k, "0")),
                     k.es(k.lt(k.call(h, {k.pv(c, 1)}), k.div(I(k, "100"), k.asgE(x, k.call(g, {k.pv(c, 0)}))))), k.ret(k.v(x))});
    k.done(c);
  };
  rh("hook_delay_write_slot", "is emitted after an item that follows it", [=](Kit& k) {
    k.lower_options.delay_write_slot = true;
    write_then_dependent_check(k, false);
  });
  lc("calls_write_delay_hook_off", "h(b) < 100 / (x = g(a)); return x: without the hook the write precedes the divisor check", [=](Kit& k) {
    write_then_dependent_check(k, true);
  });
  lc("calls_write_slot_hook_off", "(x = g(a)); return x: without the hook the write follows g", [=](Kit& k) { write_after_call(k, true); });

  // The fixed points emitted before an item are exactly its set (check_fixed_order); the hook swaps the first two fixed points.
  auto nested_with_requires = [=](Kit& k) {
    Function* g = t1(k, "g");
    Function* f1 = k.fn("f1", k.i32, {{"x", k.i32}});
    k.req(f1, k.gt(k.F(f1, 0), I(k, "0")));
    k.trusted(f1);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}});
    const Expr* call = k.call(f1, {k.call(g, {k.pv(c, 0)})});
    c->body = k.blk({k.ret(call)});
    k.done(c);
    return call;
  };
  rh("hook_perturb_fixed_order", "are not exactly the set it depends on", [=](Kit& k) {
    k.lower_options.perturb_fixed_order = true;
    nested_with_requires(k);
  });
  lc("calls_fixed_order_hook_off", "f1(g(x)) with requires x > 0 on f1: without the hook the fixed points are in order (f1's precondition may fail)", [=](Kit& k) {
    const Expr* call = nested_with_requires(k);
    k.fail("precondition", call->loc);  // checked after g returns
  });
  // The permanent check of `flush` (decision 0026 item 3): every base half of a set is contained in its exact half.  The hook clears the exact half of the
  // first `requires` obligation whose base half is not empty (f1's clause reads its argument g(x): the base half is {g}), so the check must report it.
  rh("hook_drop_exact_set", "the base half of a set of a full expression is not contained in its exact half", [=](Kit& k) {
    k.lower_options.drop_exact_set = true;
    nested_with_requires(k);
  });
  lc("calls_drop_exact_set_hook_off", "f1(g(x)) with requires x > 0 on f1: without the hook the base half is contained in the exact half (f1's precondition may fail)",
     [=](Kit& k) {
       const Expr* call = nested_with_requires(k);
       k.fail("precondition", call->loc);  // checked after g returns, exact set {g}
     });
  // The recursion components are those of the emitted methods, dead code included (silver's graph has an edge for every call).
  //   f(n): return never(0) + g(n + 1);       g(c): if (false) { f(c); } return 0;
  // f -> g -> f is a cycle only through the dead call; g(n + 1) does not decrease the measure, and never(0) comes first: the explicit
  // check, placed before never(0), is what finds it.
  auto dead_back_edge = [=](Kit& k) {
    Function* n = never(k);
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    Function* g = k.fn("g", k.i32, {{"c", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "99"));
    k.dec(f, k.F(f, 0));
    k.req(g, k.range(k.F(g, 0), k.i32, "0", "100"));
    k.dec(g, k.F(g, 0));
    const Expr* to_g = k.call(g, {k.add(k.pv(f, 0), I(k, "1"))});
    f->body = k.blk({k.ret(k.add(k.call(n, {I(k, "0")}), to_g))});
    k.done(f);
    g->body = k.blk({k.iff(k.no(), k.blk({k.es(k.call(f, {k.pv(g, 0)}))})), k.ret(I(k, "0"))});
    k.done(g);
    return to_g;
  };
  lc("calls_termination_dead_back_edge", "never(0) + g(n + 1) where the only call back is dead code: the cycle exists, the measure does not decrease, "
     "and the explicit check finds it", [=](Kit& k) {
    const Expr* to_g = dead_back_edge(k);
    k.fail_only("termination", to_g->loc, 2, 0);
  });
  rh("hook_cycles_miss_dead_code", "recursion components", [=](Kit& k) {
    k.lower_options.cycles_ignore_nested_calls = true;
    dead_back_edge(k);
  });
  // The explicit check of a recursive call is placed by the call's PRE even when the call's own set is not the first: g(h(n)) after
  // never(0).  PRE(g) = {h}, so the check comes after h and before never.
  lc("calls_termination_check_follows_its_pre", "never(0) + g(h(n)) in a recursive g: h(n) is anything, so g's measure may not decrease; the check sits after h, before never",
     [=](Kit& k) {
       Function* n = never(k);
       Function* h = t1(k, "h");
       Function* g = k.fn("g", k.i32, {{"n", k.i32}});
       k.dec(g, k.F(g, 0));
       const Expr* c = k.call(g, {k.call(h, {k.pv(g, 0)})});
       g->body = k.blk({k.ret(k.add(k.call(n, {I(k, "0")}), c))});
       k.done(g);
       k.fail_only("termination", c->loc, 2, 0);
     });
  // ---- an assert(e) with calls goes through the same lowering as code ----
  lc("calls_assert_guarded_call", "assert(a > 0 && pos(a) == a): the call is in the right operand of && inside assert", [=](Kit& k) {
    Function* p = g_id(k, true);
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.req(f, k.gt(k.F(f, 0), I(k, "0")));
    f->body = k.blk({k.asrt(k.land(k.gt(k.pv(f, 0), I(k, "0")), k.eq(k.call(p, {k.pv(f, 0)}), k.pv(f, 0))))});
    k.done(f);
  });
  lc("calls_assert_guarded_call_fails", "assert(a > 0 && pos(a) > a): pos(a) == a, so the assert fails", [=](Kit& k) {
    Function* p = g_id(k, true);
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    k.req(f, k.gt(k.F(f, 0), I(k, "0")));
    const Stmt* s = k.asrt(k.land(k.gt(k.pv(f, 0), I(k, "0")), k.gt(k.call(p, {k.pv(f, 0)}), k.pv(f, 0))));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("calls_assert_two_calls", "assert(g(1) + g(2) == 3): two unsequenced calls inside assert", [=](Kit& k) {
    Function* g = g_id(k);
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.asrt(k.eq(k.add(k.call(g, {I(k, "1")}), k.call(g, {I(k, "2")})), I(k, "3")))});
    k.done(f);
  });

  // =====================================================================================================================================
  // Exact sets (decision 0026, part A).  PRE-REGISTERED: every outcome below was written from the record before the lowering change
  // existed, and is not to be adjusted to what the code prints.
  //   * A precondition has the set PRE(c): every call in the arguments (not only the values its clauses read).
  //   * The value of a comma carries its floor: the enclosing floor and everything the left operand contains.
  //   * A full expression whose exact sets are not a chain takes the base sets (0021 item 10); only when both are not a chain is it
  //     rejected (`unsequenced_calls`).
  // The cases without a `fail` verify, the ones with it must fail at that node for that category, and nothing else may fail.
  // =====================================================================================================================================
  // `void nr`-like and `validate` helpers: `never(x)` (ensures false) and `zero(y)` (any result) are above; `validate(y)` returns only
  // when y > 0 (strong) or y >= 0 (weak); `h(x)` requires x > 0.
  auto validate_fn = [=](Kit& k, bool strong) {
    Function* v = k.fn("validate", k.vd, {{"y", k.i32}});
    k.ens(v, strong ? k.gt(k.F(v, 0), I(k, "0")) : k.ge(k.F(v, 0), I(k, "0")));
    k.trusted(v);
    return v;
  };
  auto h_positive = [=](Kit& k) {
    Function* h = k.fn("h", k.i32, {{"x", k.i32}});
    k.req(h, k.gt(k.F(h, 0), I(k, "0")));
    k.trusted(h);
    return h;
  };

  // ---- a precondition follows the calls in its arguments (item 1, first set) ----
  // int f(int a, int b) { return f1(a, inner(b)); }   f1 requires x > 0 (it reads only a).
  //   inner = nr (never returns): the precondition follows nr and is unreachable: verifies.  (Under the base sets it was checked first
  //   and reported a false alarm.)   inner = zero (returns): the precondition may fail: `precondition` at the call of f1.
  auto pre_after_argument_call = [=](Kit& k, bool inner_returns) {
    Function* inner = inner_returns ? zero(k) : never(k);
    Function* f1 = k.fn("f1", k.i32, {{"x", k.i32}, {"y", k.i32}});
    k.req(f1, k.gt(k.F(f1, 0), I(k, "0")));
    k.trusted(f1);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    const Expr* call = k.call(f1, {k.pv(f, 0), k.call(inner, {k.pv(f, 1)})});
    f->body = k.blk({k.ret(call)});
    k.done(f);
    if (inner_returns) k.fail("precondition", call->loc);
  };
  lc("exact_pre_follows_argument_call", "f1(a, nr(b)) with requires x > 0 on f1 (reading only a), nr never returning: the precondition follows nr, "
     "so it is unreachable and the function verifies", [=](Kit& k) { pre_after_argument_call(k, false); });
  lc("exact_pre_follows_argument_call_returns", "f1(a, zero(b)): the same with a returning call: a is unconstrained, `precondition` at the call of f1",
     [=](Kit& k) { pre_after_argument_call(k, true); });

  // ---- the value of a comma carries its floor (item 1, second set) ----
  // int f(int d) { return 100 / (abort(), d); }   the divisor check follows abort: verifies.  With a returning call: division-by-zero.
  auto comma_divisor = [=](Kit& k, bool returns) {
    const Expr* left = returns ? k.call(zero(k), {I(k, "0")}) : k.call(k.abort_fn(), {});
    Function* f = k.fn("f", k.i32, {{"d", k.i32}});
    const Expr* dv = k.div(I(k, "100"), k.comma(left, k.pv(f, 0)));
    f->body = k.blk({k.ret(dv)});
    k.done(f);
    if (returns) k.fail("division-by-zero", dv->loc);
  };
  lc("exact_comma_value_divisor", "100 / (abort(), d): the divisor is the value of a comma whose left operand ends the program, so its check "
     "follows abort and is unreachable: verifies", [=](Kit& k) { comma_divisor(k, false); });
  lc("exact_comma_value_divisor_returns", "100 / (zero(0), d): the left operand returns, d may be 0: `division-by-zero`",
     [=](Kit& k) { comma_divisor(k, true); });
  // bool f(int d) { return (abort(), 1) && 100 / d > 0; }   the guard is a comma value: the right operand's branch follows abort.
  auto comma_guard = [=](Kit& k, bool returns) {
    const Expr* left = returns ? k.call(zero(k), {I(k, "0")}) : k.call(k.abort_fn(), {});
    Function* f = k.fn("f", k.bl, {{"d", k.i32}});
    const Expr* dv = k.div(I(k, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.land(k.tb(k.comma(left, I(k, "1"))), k.gt(dv, I(k, "0"))))});
    k.done(f);
    if (returns) k.fail("division-by-zero", dv->loc);
  };
  lc("exact_comma_value_guard", "(abort(), 1) && 100 / d > 0: the guard is the value of a comma that ends the program, the branch follows it: verifies",
     [=](Kit& k) { comma_guard(k, false); });
  lc("exact_comma_value_guard_returns", "(zero(0), 1) && 100 / d > 0: the left operand returns, d may be 0: `division-by-zero`",
     [=](Kit& k) { comma_guard(k, true); });

  // ---- the idiom: h((validate(y), y)) ----
  // int c(int y) { return h((validate(y), y)); }   validate returns only when y > 0 (strong) or y >= 0 (weak); h requires x > 0.
  auto validate_idiom = [=](Kit& k, bool strong, bool force_base = false) {
    k.lower_options.force_base_sets = force_base;
    Function* v = validate_fn(k, strong);
    Function* h = h_positive(k);
    Function* c = k.fn("c", k.i32, {{"y", k.i32}});
    const Expr* hc = k.call(h, {k.comma(k.call(v, {k.pv(c, 0)}), k.pv(c, 0))});
    c->body = k.blk({k.ret(hc)});
    k.done(c);
    if (!strong || force_base) k.fail("precondition", hc->loc);
  };
  lc("exact_validate_idiom", "h((validate(y), y)) with validate(y) returning only when y > 0 and requires x > 0 on h: the precondition waits for validate",
     [=](Kit& k) { validate_idiom(k, true); });
  lc("exact_validate_idiom_force_base", "the same strong validate with the hook force_base_sets (every full expression takes today's sets): h's precondition "
     "is checked before validate and fails, `precondition` at the call of h: step 1 is what removes the false alarm", [=](Kit& k) { validate_idiom(k, true, true); });
  lc("exact_validate_idiom_weak", "the same with validate returning when y >= 0: y == 0 reaches h, `precondition` at the call of h",
     [=](Kit& k) { validate_idiom(k, false); });
  // ... and inside a branch: int c(bool a, int y) { return a && h((validate(y), y)) > 0; }   the branch's own full expression uses exact sets.
  auto validate_in_branch = [=](Kit& k, bool strong) {
    Function* v = validate_fn(k, strong);
    Function* h = h_positive(k);
    Function* c = k.fn("c", k.i32, {{"a", k.bl}, {"y", k.i32}});
    const Expr* hc = k.call(h, {k.comma(k.call(v, {k.pv(c, 1)}), k.pv(c, 1))});
    c->body = k.blk({k.ret(k.bi(k.i32, k.land(k.pv(c, 0), k.gt(hc, I(k, "0")))))});
    k.done(c);
    if (!strong) k.fail("precondition", hc->loc);
  };
  lc("exact_validate_idiom_in_branch", "a && h((validate(y), y)) > 0: the call is in the right operand of &&, whose own full expression uses exact sets",
     [=](Kit& k) { validate_in_branch(k, true); });
  lc("exact_validate_idiom_in_branch_weak", "the same with the weak validate: `precondition` at the call of h", [=](Kit& k) { validate_in_branch(k, false); });

  // ---- one `requires` clause per argument: now a lowering case (it was `unsequenced_calls`) ----
  // int c(int x, int y) { return take(fa(x), hb(y)); }   take requires 1 <= a and 2 <= b; each clause now has the set {fa, hb}.
  auto take_split = [=](Kit& k, bool both_hold) {
    Function* fa = k.fn("fa", k.i32, {{"x", k.i32}});
    k.ens(fa, k.ge_c(k.R(fa), k.i32, "1"));
    k.trusted(fa);
    Function* hb = k.fn("hb", k.i32, {{"x", k.i32}});
    if (both_hold) k.ens(hb, k.ge_c(k.R(hb), k.i32, "2"));
    k.trusted(hb);
    Function* take = k.fn("take", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(take, k.le(I(k, "1"), k.F(take, 0)));
    k.req(take, k.le(I(k, "2"), k.F(take, 1)));
    k.trusted(take);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}});
    const Expr* call = k.call(take, {k.call(fa, {k.pv(c, 0)}), k.call(hb, {k.pv(c, 1)})});
    c->body = k.blk({k.ret(call)});
    k.done(c);
    // two clauses at the call; only the second (b) can fail, fa's result is >= 1
    if (!both_hold) k.fail_only("precondition", call->loc, 2, 1);
  };
  lc("exact_split_requires", "take(fa(x), hb(y)) with requires 1 <= a and requires 2 <= b, fa >= 1 and hb >= 2: both clauses wait for both calls "
     "(accepted now, was `unsequenced_calls`): verifies", [=](Kit& k) { take_split(k, true); });
  lc("exact_split_requires_fails", "the same with no postcondition on hb: the second clause (b) may fail, `precondition` at the call of take",
     [=](Kit& k) { take_split(k, false); });

  // ---- the fallback: exact sets that are not a chain use the base sets (item 2, step 2) ----
  // int c(int x, int y, int z) { return h2(f(x), y) + 100 / g(z); }   h2 requires y > 0 (reads only its second parameter).
  //   exact: PRE(h2) = {f} against {g}: not a chain.  base: {} against {g}: a chain, accepted.
  auto fallback_pre = [=](Kit& k, bool y_positive) {
    Function* f_ = t1(k, "f");
    Function* g_ = k.fn("g", k.i32, {{"z", k.i32}});
    k.ens(g_, k.range(k.R(g_), k.i32, "1", "100"));
    k.trusted(g_);
    Function* h2 = k.fn("h2", k.i32, {{"x", k.i32}, {"y", k.i32}});
    k.req(h2, k.gt(k.F(h2, 1), I(k, "0")));
    k.ens(h2, k.range(k.R(h2), k.i32, "0", "1000"));
    k.trusted(h2);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}, {"z", k.i32}});
    if (y_positive) k.req(c, k.gt(k.F(c, 1), I(k, "0")));
    const Expr* hc = k.call(h2, {k.call(f_, {k.pv(c, 0)}), k.pv(c, 1)});
    c->body = k.blk({k.ret(k.add(hc, k.div(I(k, "100"), k.call(g_, {k.pv(c, 2)}))))});
    k.done(c);
    if (!y_positive) k.fail("precondition", hc->loc);
  };
  lc("exact_fallback_precondition", "h2(f(x), y) + 100 / g(z), h2 requiring y > 0, with requires y > 0 on c: the exact sets are not a chain, the base "
     "sets are, so it is accepted: verifies", [=](Kit& k) { fallback_pre(k, true); });
  lc("exact_fallback_precondition_fails", "the same without requires on c: y may be 0, `precondition` at the call of h2",
     [=](Kit& k) { fallback_pre(k, false); });
  // int c(int x, int y, int z) { return 100 / (f(x), y) + 100 / g(z); }   the comma's value carries f in the exact sets: {f} against {g}; base:
  // {} against {g}.
  auto fallback_comma = [=](Kit& k, bool y_positive) {
    Function* f_ = t1(k, "f");
    Function* g_ = k.fn("g", k.i32, {{"z", k.i32}});
    k.ens(g_, k.range(k.R(g_), k.i32, "1", "100"));
    k.trusted(g_);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}, {"z", k.i32}});
    if (y_positive) k.req(c, k.gt(k.F(c, 1), I(k, "0")));
    const Expr* d1 = k.div(I(k, "100"), k.comma(k.call(f_, {k.pv(c, 0)}), k.pv(c, 1)));
    c->body = k.blk({k.ret(k.add(d1, k.div(I(k, "100"), k.call(g_, {k.pv(c, 2)}))))});
    k.done(c);
    if (!y_positive) k.fail("division-by-zero", d1->loc);
  };
  lc("exact_fallback_comma_value", "100 / (f(x), y) + 100 / g(z) with requires y > 0: the exact sets are not a chain, the base sets are: accepted, verifies",
     [=](Kit& k) { fallback_comma(k, true); });
  lc("exact_fallback_comma_value_fails", "the same without requires on c: `division-by-zero` at the first division", [=](Kit& k) { fallback_comma(k, false); });

  // ---- one full expression in each step: the inner one (a branch's) takes step 1, the outer one takes step 2 ----
  // int c(int x, int y, int z, int w) requires y > 0 { return h2(f(x), y) + (g(z) > 0 && h((validate(w), w)) > 0); }
  //   outer: PRE(h2) is {f} (exact) and D(B) of the branch is {g, B}: not a chain; the base sets ({} and {g, B}) are, so the outer expression
  //   takes step 2 and is accepted.  inner (the right operand of &&, its own full expression): h's precondition has the exact set {validate},
  //   a chain, so it takes step 1 and waits for validate.  With the strong validate the inner precondition holds only under exact sets: verifies.
  //   With the weak validate (y >= 0): `precondition` at the call of h.
  auto mixed_steps = [=](Kit& k, bool strong) {
    Function* f_ = t1(k, "f");
    Function* g_ = t1(k, "g");
    Function* v = validate_fn(k, strong);
    Function* h = h_positive(k);
    Function* h2 = k.fn("h2", k.i32, {{"x", k.i32}, {"y", k.i32}});
    k.req(h2, k.gt(k.F(h2, 1), I(k, "0")));
    k.ens(h2, k.range(k.R(h2), k.i32, "0", "1000"));
    k.trusted(h2);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}, {"z", k.i32}, {"w", k.i32}});
    k.req(c, k.gt(k.F(c, 1), I(k, "0")));
    const Expr* hc = k.call(h, {k.comma(k.call(v, {k.pv(c, 3)}), k.pv(c, 3))});
    const Expr* branch = k.bi(k.i32, k.land(k.gt(k.call(g_, {k.pv(c, 2)}), I(k, "0")), k.gt(hc, I(k, "0"))));
    c->body = k.blk({k.ret(k.add(k.call(h2, {k.call(f_, {k.pv(c, 0)}), k.pv(c, 1)}), branch))});
    k.done(c);
    if (!strong) k.fail("precondition", hc->loc);
  };
  lc("exact_mixed_steps", "h2(f(x), y) + (g(z) > 0 && h((validate(w), w)) > 0): the outer expression takes the base sets (the fallback), the branch's own "
     "expression the exact sets, so h's precondition waits for validate: verifies", [=](Kit& k) { mixed_steps(k, true); });
  lc("exact_mixed_steps_weak", "the same with the weak validate: `precondition` at the call of h", [=](Kit& k) { mixed_steps(k, false); });

  // ---- the pins: a set computed too large would verify these (it is the unsound direction, invisible to the checks, item 4) ----
  // nr(0) + h((g(y), y)): PRE(h) is {g}, not {nr}: the precondition is checked before nr and fails (y is anything after g).
  auto pin_pre = [=](Kit& k, bool validated) {
    Function* n = never(k);
    Function* h = h_positive(k);
    Function* g = validated ? validate_fn(k, true) : t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"y", k.i32}});
    const Expr* hc = k.call(h, {k.comma(k.call(g, {k.pv(c, 0)}), k.pv(c, 0))});
    c->body = k.blk({k.ret(k.add(k.call(n, {I(k, "0")}), hc))});
    k.done(c);
    if (!validated) k.fail("precondition", hc->loc);
  };
  lc("exact_pin_pre_not_after_sibling", "nr(0) + h((g(y), y)): PRE(h) is {g}, not nr, so the precondition is checked before nr and fails: `precondition`",
     [=](Kit& k) { pin_pre(k, false); });
  lc("exact_pin_pre_not_after_sibling_validated", "nr(0) + h((validate(y), y)) with validate returning only for y > 0: the precondition holds after validate: verifies",
     [=](Kit& k) { pin_pre(k, true); });
  // nr(0) + 100 / (g(y), y): the comma's value carries {g}, not {nr}: the divisor check precedes nr and fails.
  auto pin_comma = [=](Kit& k, bool validated) {
    Function* n = never(k);
    Function* g = validated ? validate_fn(k, true) : t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"y", k.i32}});
    const Expr* d = k.div(I(k, "100"), k.comma(k.call(g, {k.pv(c, 0)}), k.pv(c, 0)));
    c->body = k.blk({k.ret(k.add(k.call(n, {I(k, "0")}), d))});
    k.done(c);
    if (!validated) k.fail("division-by-zero", d->loc);
  };
  lc("exact_pin_comma_not_after_sibling", "nr(0) + 100 / (g(y), y): the comma's value carries {g}, not nr: the divisor check precedes nr and fails: "
     "`division-by-zero`", [=](Kit& k) { pin_comma(k, false); });
  lc("exact_pin_comma_not_after_sibling_validated", "nr(0) + 100 / (validate(y), y) with validate returning only for y > 0: verifies",
     [=](Kit& k) { pin_comma(k, true); });
  // (nr(0), 1) + 100 / d must fail division-by-zero: the floor does not leak to a sibling.  fx_comma_floor_does_not_leak already pins it.
}

// ---- shapes whose checks do not form a chain are refused by lowering (rl) ----
void define_calls_in_expressions_rejections() {
  auto I = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto t1 = [](Kit& k, const char* name, Type ret = Type()) {
    Function* t = k.fn(name, ret.is_void() ? k.i32 : ret, {{"x", k.i32}});
    k.trusted(t);
    return t;
  };
  const char* kUnordered = "are not ordered";
  rl("calls_rej_two_divisions", "the calls to \"f\" and \"g\" are not ordered", [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}});
    c->body = k.blk({k.ret(k.add(k.div(I(k, "100"), k.call(f, {k.pv(c, 0)})), k.div(I(k, "100"), k.call(g, {k.pv(c, 1)}))))});
    k.done(c);
  });
  rl("calls_rej_comma_then_division", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* h = t1(k, "h");
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}, {"z", k.i32}});
    const Expr* left = k.comma(k.call(f, {k.pv(c, 0)}), k.div(I(k, "100"), k.call(g, {k.pv(c, 1)})));
    c->body = k.blk({k.ret(k.add(left, k.div(I(k, "100"), k.call(h, {k.pv(c, 2)}))))});
    k.done(c);
  });
  rl("calls_rej_two_branches", kUnordered, [=](Kit& k) {
    Function* g = t1(k, "gb", k.bl);
    Function* h = t1(k, "hb", k.bl);
    Function* c = k.fn("c", k.i32, {{"a", k.bl}, {"b", k.bl}, {"x", k.i32}, {"y", k.i32}});
    c->body = k.blk({k.ret(k.add(k.bi(k.i32, k.land(k.pv(c, 0), k.call(g, {k.pv(c, 2)}))), k.bi(k.i32, k.land(k.pv(c, 1), k.call(h, {k.pv(c, 3)})))))});
    k.done(c);
  });
  rl("calls_rej_inside_a_branch", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"a", k.bl}, {"x", k.i32}, {"y", k.i32}});
    const Expr* inner = k.gt(k.add(k.div(I(k, "100"), k.call(f, {k.pv(c, 1)})), k.div(I(k, "100"), k.call(g, {k.pv(c, 2)}))), I(k, "0"));
    c->body = k.blk({k.ret(k.bi(k.i32, k.land(k.pv(c, 0), inner)))});
    k.done(c);
  });
  rl("calls_rej_guard_counts", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}, {"z", k.i32}});
    const Expr* guarded = k.land(k.gt(k.call(f, {k.pv(c, 0)}), I(k, "0")), k.gt(k.div(I(k, "100"), k.pv(c, 1)), I(k, "0")));
    c->body = k.blk({k.ret(k.add(k.bi(k.i32, guarded), k.div(I(k, "100"), k.call(g, {k.pv(c, 2)}))))});
    k.done(c);
  });
  rl("calls_rej_short_parameters", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* h = t1(k, "h");
    Function* g = k.fn("g", k.i16, {{"p", k.i16}, {"q", k.i16}});
    k.trusted(g);
    Function* c = k.fn("c", k.i16, {{"x", k.i32}, {"y", k.i32}});
    c->body = k.blk({k.ret(k.call(g, {k.cv(k.i16, k.call(f, {k.pv(c, 0)})), k.cv(k.i16, k.call(h, {k.pv(c, 1)}))}))});
    k.done(c);
  });
  // Decision 0025 item 2: two writes no longer conflict (calls_rej_two_writes is retired).  What stays rejected: the checks of flagged
  // snapshots and a branch with a write are in F.
  rl("calls_rej_flagged_snapshots", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.vd, {{"a", k.i32}, {"b", k.i32}});
    Var* x = k.local(c, "x", k.i32);
    Var* y = k.local(c, "y", k.i32);
    c->body = k.blk({k.decl(x, nullptr), k.decl(y, nullptr),
                     k.es(k.add(k.comma(k.asgE(x, k.call(f, {k.pv(c, 0)})), k.v(x)), k.comma(k.asgE(y, k.call(g, {k.pv(c, 1)})), k.v(y))))});
    k.done(c);
  });
  rl("calls_rej_branch_with_write", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"b", k.i32}});
    Var* x = k.local(c, "x", k.i32);
    const Expr* branch = k.bi(k.i32, k.land(k.gt(k.call(f, {k.pv(c, 0)}), I(k, "0")), k.ne(k.asgE(x, I(k, "1")), I(k, "0"))));
    c->body = k.blk({k.decl(x, I(k, "0")), k.ret(k.add(branch, k.div(I(k, "100"), k.call(g, {k.pv(c, 1)}))))});
    k.done(c);
  });
  rl("calls_rej_loop_condition", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.vd, {{"x", k.i32}, {"y", k.i32}});
    Clause m = k.cl(I(k, "0"));
    const Expr* cnd = k.gt(k.add(k.div(I(k, "100"), k.call(f, {k.pv(c, 0)})), k.div(I(k, "100"), k.call(g, {k.pv(c, 1)}))), I(k, "0"));
    c->body = k.blk({k.loop(cnd, {}, {m}, k.blk({}), nullptr, LoopForm::While, k.jid())});
    k.done(c);
  });
  rl("calls_rej_in_an_assert", kUnordered, [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.vd, {{"x", k.i32}, {"y", k.i32}});
    c->body = k.blk({k.asrt(k.gt(k.add(k.div(I(k, "100"), k.call(f, {k.pv(c, 0)})), k.div(I(k, "100"), k.call(g, {k.pv(c, 1)}))), I(k, "0")))});
    k.done(c);
  });
  // A conflict that comes from a recursive call's termination check is worded by that call (decision 0021 item 3): nothing visible
  // depends on g alone in `f(g(n - 1), y) + 100 / h(y)`, only the check that f's measure decreases.
  rl("calls_rej_termination_check_conflict", "the call to \"h\" and the recursive call to \"f\" (its termination check needs \"g\" first) are not ordered",
     [=](Kit& k) {
       Function* g = t1(k, "g");
       Function* h = t1(k, "h");
       Function* f = k.fn("f", k.i32, {{"x", k.i32}, {"y", k.i32}});
       k.dec(f, k.F(f, 0));
       const Expr* rec = k.call(f, {k.call(g, {k.sub(k.pv(f, 0), I(k, "1"))}), k.pv(f, 1)});
       f->body = k.blk({k.ret(k.add(rec, k.div(I(k, "100"), k.call(h, {k.pv(f, 1)}))))});
       k.done(f);
     });
  // Decision 0026 (exact sets): a rejection stays where both steps (the exact sets and the base sets) are not a chain.  PRE-REGISTERED.
  //   100 / (u(a), f(x)) + 100 / g(y): the first divisor is a call's result, and PRE(f) contains the floor {u} in both halves: {u, f} (exact and
  //   base) against {g}: no chain either way.
  rl("calls_rej_comma_value_both_steps", kUnordered, [=](Kit& k) {
    Function* u = t1(k, "u");
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"a", k.i32}, {"x", k.i32}, {"y", k.i32}});
    const Expr* first = k.div(I(k, "100"), k.comma(k.call(u, {k.pv(c, 0)}), k.call(f, {k.pv(c, 1)})));
    c->body = k.blk({k.ret(k.add(first, k.div(I(k, "100"), k.call(g, {k.pv(c, 2)}))))});
    k.done(c);
  });
  //   take(fa(x), hb(y)) + 100 / g(z), take with one requires per argument: exact: {fa, hb} against {g}; base: {fa}, {hb} and {g}: no chain either way.
  rl("calls_rej_split_requires_with_third_call", kUnordered, [=](Kit& k) {
    Function* fa = t1(k, "fa");
    Function* hb = t1(k, "hb");
    Function* g = t1(k, "g");
    Function* take = k.fn("take", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(take, k.le(I(k, "1"), k.F(take, 0)));
    k.req(take, k.le(I(k, "2"), k.F(take, 1)));
    k.trusted(take);
    Function* c = k.fn("c", k.i32, {{"x", k.i32}, {"y", k.i32}, {"z", k.i32}});
    const Expr* call = k.call(take, {k.call(fa, {k.pv(c, 0)}), k.call(hb, {k.pv(c, 1)})});
    c->body = k.blk({k.ret(k.add(call, k.div(I(k, "100"), k.call(g, {k.pv(c, 2)}))))});
    k.done(c);
  });
  // The message names both calls and the position is the later one's.
  rl("calls_rej_message_names_both", "the calls to \"f\" and \"g\" are not ordered", [=](Kit& k) {
    Function* f = t1(k, "f");
    Function* g = t1(k, "g");
    Function* c = k.fn("c", k.i32, {{"x", k.i32}});
    c->body = k.blk({k.ret(k.add(k.div(I(k, "1"), k.call(f, {k.pv(c, 0)})), k.div(I(k, "1"), k.call(g, {k.pv(c, 0)}))))});
    k.done(c);
  });
}

}  // namespace
