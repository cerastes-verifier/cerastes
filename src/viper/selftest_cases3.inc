// (included by selftest.cpp) Side effects used as values and sequencing (decision 0020): assignments, compound assignments,
// increments and the comma operator inside expressions, in code and in loop conditions; the sequencing rule; and the typing
// rejections of the new nodes.  Programs without these nodes are not touched by anything here.
namespace {

void define_side_effects() {
  using LF = LoopForm;
  auto I = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  // f(n) with n in [0, 100].
  auto fn_n = [](Kit& k, Type ret) {
    Function* f = k.fn("f", ret, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    return f;
  };
  auto plus1 = [](Kit& k, Var* x) { return k.asg(x, k.add(k.v(x), k.num(k.i32, "1"))); };
  // A trusted `int g(int x)` with `ensures r == x` (and, when `positive`, `requires x > 0`).
  auto g_id = [](Kit& k, bool positive) {
    Function* g = k.fn("g", k.i32, {{"x", k.i32}});
    if (positive) k.req(g, k.gt(k.F(g, 0), k.num(k.i32, "0")));
    k.ens(g, k.eq(k.R(g), k.F(g, 0)));
    k.trusted(g);
    return g;
  };
  // A trusted `void v(void)`.
  auto v_void = [](Kit& k) {
    Function* v = k.fn("v", k.vd);
    k.trusted(v);
    return v;
  };

  // A case and its twin: `build(k, wrong)` builds the program with a postcondition that is right, or wrong, and returns the `return`
  // statement the postcondition is asserted at (invariant 2: each accepted shape pins its value by a twin that fails).
  auto twin = [](const char* name, const char* doc, std::function<const Stmt*(Kit&, bool)> build) {
    lc(name, doc, [build](Kit& k) { build(k, false); });
    lc(std::string(name) + "_wrong", std::string(doc) + " (with a wrong postcondition)", [build](Kit& k) {
      const Stmt* r = build(k, true);
      k.fail("postcondition", r->loc);
    });
  };

  // ---- assignments used as values ----
  auto assign_value = [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.mul(I(k, "2"), k.F(f, 0)), I(k, wrong ? "0" : "1"))));
    const Stmt* r = k.ret(k.add(k.v(y), k.v(x)));
    f->body = k.blk({k.decl(x, I(k, "0")), k.decl(y, k.add(k.asgE(x, k.pv(f, 0)), I(k, "1"))), r});
    k.done(f);
    if (wrong) k.fail("postcondition", r->loc);
  };
  lc("fx_assign_value", "int y = (x = n) + 1; return y + x: the value of the assignment is n, x is n afterwards", [=](Kit& k) { assign_value(k, false); });
  lc("fx_assign_value_wrong", "the same with a postcondition that forgets the + 1", [=](Kit& k) { assign_value(k, true); });
  auto chained = [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.mul(I(k, wrong ? "3" : "2"), k.F(f, 0))));
    const Stmt* r = k.ret(k.add(k.v(x), k.v(y)));
    f->body = k.blk({k.decl(x, I(k, "0")), k.decl(y, I(k, "0")), k.asg(y, k.asgE(x, k.pv(f, 0))), r});
    k.done(f);
    if (wrong) k.fail("postcondition", r->loc);
  };
  lc("fx_assign_chained", "y = x = n: both are n", [=](Kit& k) { chained(k, false); });
  lc("fx_assign_chained_wrong", "y = x = n with a postcondition of 3 * n", [=](Kit& k) { chained(k, true); });
  auto narrow = [=](Kit& k, bool bounded) {
    Function* f = k.fn("f", k.i32, {{"v", k.i32}});
    if (bounded) k.req(f, k.range(k.F(f, 0), k.i32, "-100", "100"));
    Var* s = k.local(f, "s", k.i16);
    Var* i = k.local(f, "i", k.i32);
    const Expr* c = k.cv(k.i16, k.pv(f, 0));
    if (bounded) k.ens(f, k.eq(k.R(f), k.mul(I(k, "2"), k.F(f, 0))));
    f->body = k.blk({k.decl(s, k.num(k.i16, "0")), k.decl(i, I(k, "0")), k.asg(i, k.cv(k.i32, k.asgE(s, c))),
                     k.ret(k.add(k.v(i), k.cv(k.i32, k.v(s))))});
    k.done(f);
    if (!bounded) k.fail("conversion", c->loc);
  };
  lc("fx_assign_chain_narrowing", "i = s = v with a short s: the conversion of v to short fits for v in [-100, 100]", [=](Kit& k) { narrow(k, true); });
  lc("fx_assign_chain_narrowing_over", "the same for any int v: the conversion to short may not fit", [=](Kit& k) { narrow(k, false); });
  lc("fx_assign_bool", "bool b; return (b = (n > 5)) as an int: the value of the assignment is the stored bool", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* b = k.local(f, "b", k.bl);
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "5")), I(k, "1"), I(k, "0"))));
    f->body = k.blk({k.decl(b, k.no()), k.ret(k.bi(k.i32, k.asgE(b, k.gt(k.pv(f, 0), I(k, "5")))))});
    k.done(f);
  });
  lc("fx_assign_bool_wrong", "the same with the postcondition of n > 6", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* b = k.local(f, "b", k.bl);
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "6")), I(k, "1"), I(k, "0"))));
    const Stmt* r = k.ret(k.bi(k.i32, k.asgE(b, k.gt(k.pv(f, 0), I(k, "5")))));
    f->body = k.blk({k.decl(b, k.no()), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });

  // ---- compound assignments used as values ----
  lc("fx_compound_value", "int y = (x += 2) * 3; return y + x with x = n: 4n + 8", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.mul(I(k, "4"), k.F(f, 0)), I(k, "8"))));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, k.mul(k.cmpE(x, BinaryOp::Add, I(k, "2")), I(k, "3"))), k.ret(k.add(k.v(y), k.v(x)))});
    k.done(f);
  });
  lc("fx_compound_value_wrong", "the same with 4n + 6", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.mul(I(k, "4"), k.F(f, 0)), I(k, "6"))));
    const Stmt* r = k.ret(k.add(k.v(y), k.v(x)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, k.mul(k.cmpE(x, BinaryOp::Add, I(k, "2")), I(k, "3"))), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });
  lc("fx_compound_overflow", "y = (x += 1) with any int x: overflows at INT_MAX", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    const Expr* e = k.cmpE(x, BinaryOp::Add, I(k, "1"));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(e)});
    k.done(f);
    k.fail("overflow", e->loc);
  });
  lc("fx_compound_div_zero", "return x /= d: d may be zero, and INT_MIN / -1 overflows", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"d", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    const Expr* e = k.cmpE(x, BinaryOp::Div, k.pv(f, 1));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(e)});
    k.done(f);
    k.fail("division-by-zero", e->loc);
    k.fail("overflow", e->loc);
  });
  lc("fx_compound_div_ok", "return x /= d with d in [1, 10]", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"d", k.i32}});
    k.req(f, k.range(k.F(f, 1), k.i32, "1", "10"));
    Var* x = k.local(f, "x", k.i32);
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(k.cmpE(x, BinaryOp::Div, k.pv(f, 1)))});
    k.done(f);
  });

  // ---- increments and decrements used as values ----
  // y = x++ (post, old value), y = ++x (pre, new value), y = x-- and y = --x; `x` starts as n; returns y + x.
  auto incdec = [=](Kit& k, bool up, bool post, const char* ens_const) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    // post: y = n, x = n +- 1; pre: y = x = n +- 1
    const Expr* sum = k.add(k.mul(I(k, "2"), k.F(f, 0)), I(k, ens_const));
    k.ens(f, k.eq(k.R(f), sum));
    const Stmt* r = k.ret(k.add(k.v(y), k.v(x)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, I(k, "0")), k.asg(y, up ? k.incE(x, post) : k.decE(x, post)), r});
    k.done(f);
    return r;
  };
  lc("fx_incdec_post_incr", "y = x++: y is the old value, x the new one: y + x == 2n + 1", [=](Kit& k) { incdec(k, true, true, "1"); });
  lc("fx_incdec_pre_incr", "y = ++x: y + x == 2n + 2", [=](Kit& k) { incdec(k, true, false, "2"); });
  lc("fx_incdec_post_decr", "y = x--: y + x == 2n - 1", [=](Kit& k) { incdec(k, false, true, "-1"); });
  lc("fx_incdec_pre_decr", "y = --x: y + x == 2n - 2", [=](Kit& k) { incdec(k, false, false, "-2"); });
  lc("fx_incdec_post_decr_wrong", "y = x-- with the postcondition of --x", [=](Kit& k) {
    const Stmt* r = incdec(k, false, true, "-2");
    k.fail("postcondition", r->loc);
  });
  lc("fx_incdec_pre_decr_wrong", "y = --x with the postcondition of x--", [=](Kit& k) {
    const Stmt* r = incdec(k, false, false, "-1");
    k.fail("postcondition", r->loc);
  });
  lc("fx_incdec_post_incr_wrong", "y = x++ with the postcondition of ++x", [=](Kit& k) {
    const Stmt* r = incdec(k, true, true, "2");
    k.fail("postcondition", r->loc);
  });
  lc("fx_incdec_pre_incr_wrong", "y = ++x with the postcondition of x++", [=](Kit& k) {
    const Stmt* r = incdec(k, true, false, "1");
    k.fail("postcondition", r->loc);
  });
  lc("fx_incdec_overflow", "return x++ with any int x: overflows at INT_MAX", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    const Expr* e = k.incE(x);
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(e)});
    k.done(f);
    k.fail("overflow", e->loc);
  });
  lc("fx_incdec_underflow", "return --x with any int x: overflows at INT_MIN", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    const Expr* e = k.decE(x, false);
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(e)});
    k.done(f);
    k.fail("overflow", e->loc);
  });
  lc("fx_return_incdec", "int x = n; return x++: the result is n", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(k.incE(x))});
    k.done(f);
  });
  lc("fx_return_incdec_wrong", "int x = n; return ++x with the postcondition r == n", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    const Stmt* r = k.ret(k.incE(x, false));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });
  auto unsigned_wrap = [=](Kit& k, bool wrong) {
    Function* f = k.fn("f", k.vd);
    Var* x = k.local(f, "x", k.u32);
    Var* y = k.local(f, "y", k.u32);
    const Stmt* c = k.chk(k.eq_c(k.v(x), k.u32, wrong ? "4294967295" : "0"));
    f->body = k.blk({k.decl(x, k.num(k.u32, "4294967295")), k.decl(y, k.incE(x)), k.chk(k.eq_c(k.v(y), k.u32, "4294967295")), c});
    k.done(f);
    if (wrong) k.fail("check", c->loc);
  };
  lc("fx_incdec_unsigned_wraps", "unsigned x = UINT_MAX; y = x++: y is UINT_MAX and x wraps to 0", [=](Kit& k) { unsigned_wrap(k, false); });
  lc("fx_incdec_unsigned_wraps_wrong", "the same with check(x == UINT_MAX) afterwards", [=](Kit& k) { unsigned_wrap(k, true); });
  auto small = [=](Kit& k, bool bounded) {
    Function* f = k.fn("f", k.i8, {{"c", k.i8}});
    if (bounded) k.req(f, k.le_c(k.F(f, 0), k.i8, "126"));
    Var* x = k.local(f, "x", k.i8);
    const Expr* e = k.incE(x);
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(e)});
    k.done(f);
    if (!bounded) k.fail("conversion", e->loc);
  };
  lc("fx_incdec_char_ok", "signed char x; return x++ with x <= 126: the new value fits", [=](Kit& k) { small(k, true); });
  lc("fx_incdec_char_over", "the same for x == 127: the stored value does not fit a signed char", [=](Kit& k) { small(k, false); });
  auto argument = [=](Kit& k, bool positive) {
    Function* g = g_id(k, true);
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, positive ? "1" : "0", "100"));
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    const Expr* c = k.call(g, {k.incE(x)});
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, c), k.chk(k.eq(k.v(x), k.add(k.pv(f, 0), I(k, "1")))), k.ret(k.v(y))});
    k.done(f);
    if (!positive) k.fail("precondition", c->loc);
  };
  lc("fx_incdec_argument", "g(x++): the callee gets the old value (positive), x is one more afterwards", [=](Kit& k) { argument(k, true); });
  lc("fx_incdec_argument_pre", "the same with n possibly 0: the old value may violate the callee's requires", [=](Kit& k) { argument(k, false); });
  auto switch_inc = [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* r = k.local(f, "r", k.i32);
    const int id = k.jid();
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.i32, "0"), k.eq_c(k.R(f), k.i32, "10")));
    k.ens(f, k.implies(k.ne(k.F(f, 0), I(k, "0")), k.eq(k.R(f), wrong ? k.F(f, 0) : k.add(k.F(f, 0), I(k, "1")))));
    const Stmt* body = k.blk({k.cs(I(k, "0")), k.asg(r, I(k, "10")), k.brk(id), k.dflt(), k.asg(r, k.v(x))});
    const Stmt* rt = k.ret(k.v(r));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(r, I(k, "0")), k.sw(k.incE(x), body, id), rt});
    k.done(f);
    if (wrong) k.fail_only("postcondition", rt->loc, 2, 1);  // two ensures clauses; the second is the one that fails
  };
  lc("fx_switch_incdec", "switch (x++): the cases compare the old value; the default sees the new one", [=](Kit& k) { switch_inc(k, false); });
  lc("fx_switch_incdec_wrong", "the same with a postcondition that expects the old value in the default", [=](Kit& k) { switch_inc(k, true); });

  // ---- the comma operator ----
  lc("fx_comma_value", "return (x = n, x + 1): the value is that of the right operand, after the assignment", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), I(k, "1"))));
    f->body = k.blk({k.decl(x, I(k, "0")), k.ret(k.comma(k.asgE(x, k.pv(f, 0)), k.add(k.v(x), I(k, "1"))))});
    k.done(f);
  });
  lc("fx_comma_value_wrong", "the same with the postcondition r == n", [=](Kit& k) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    const Stmt* r = k.ret(k.comma(k.asgE(x, k.pv(f, 0)), k.add(k.v(x), I(k, "1"))));
    f->body = k.blk({k.decl(x, I(k, "0")), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });
  twin("fx_comma_void_call_left", "y = (v(), i++): a void call as the left operand, then the increment", [=](Kit& k, bool wrong) {
    Function* v = v_void(k);
    Function* f = fn_n(k, k.i32);
    Var* i = k.local(f, "i", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.mul(I(k, "2"), k.F(f, 0)), I(k, wrong ? "2" : "1"))));
    const Stmt* r = k.ret(k.add(k.v(y), k.v(i)));
    f->body = k.blk({k.decl(i, k.pv(f, 0)), k.decl(y, k.comma(k.call(v, {}), k.incE(i))), r});
    k.done(f);
    return r;
  });
  auto comma_calls = [=](Kit& k, bool wrong) {
    Function* h = k.fn("h", k.i32, {{"x", k.i32}});
    k.req(h, k.range(k.F(h, 0), k.i32, "0", "100"));
    k.ens(h, k.eq(k.R(h), k.add(k.F(h, 0), I(k, "1"))));
    k.trusted(h);
    Function* f = k.fn("f", k.vd);
    Var* y = k.local(f, "y", k.i32);
    const Stmt* c = k.chk(k.eq(k.v(y), I(k, wrong ? "2" : "3")));
    f->body = k.blk({k.decl(y, k.comma(k.call(h, {I(k, "1")}), k.call(h, {I(k, "2")}))), c});
    k.done(f);
    if (wrong) k.fail("check", c->loc);
  };
  lc("fx_comma_calls_sequenced", "y = (h(1), h(2)): two calls sequenced by the comma, in order: y == 3", [=](Kit& k) { comma_calls(k, false); });
  lc("fx_comma_calls_sequenced_wrong", "the same with check(y == 2)", [=](Kit& k) { comma_calls(k, true); });
  lc("fx_comma_call_precondition_after_first", "(g(0), g(1)) with g requiring x > 0: only the first call's precondition fails", [=](Kit& k) {
    Function* g = g_id(k, true);
    Function* f = k.fn("f", k.i32);
    const Expr* first = k.call(g, {I(k, "0")});
    f->body = k.blk({k.ret(k.comma(first, k.call(g, {I(k, "1")})))});
    k.done(f);
    k.fail("precondition", first->loc);
  });
  lc("fx_comma_noreturn_hides_division", "return (abort(), 1 / d): the division check follows the call that ends the program, so it holds for any d", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = k.fn("f", k.i32, {{"d", k.i32}});
    f->body = k.blk({k.ret(k.comma(k.call(a, {}), k.div(I(k, "1"), k.pv(f, 0))))});
    k.done(f);
  });
  lc("fx_comma_returning_call_division", "return (v(), 1 / d): v returns, so d may be zero", [=](Kit& k) {
    Function* v = v_void(k);
    Function* f = k.fn("f", k.i32, {{"d", k.i32}});
    const Expr* d = k.div(I(k, "1"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.comma(k.call(v, {}), d))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("fx_comma_floor_does_not_leak", "return (abort(), 0) + 100 / d: the other operand of + is unsequenced with the comma, so its check comes first", [=](Kit& k) {
    Function* a = k.abort_fn();
    Function* f = k.fn("f", k.i32, {{"d", k.i32}});
    const Expr* d = k.div(I(k, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.add(k.comma(k.call(a, {}), I(k, "0")), d))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });

  // ---- conditions with effects ----
  auto cond_write = [=](Kit& k, bool read_before) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.sub(k.F(f, 0), I(k, "1"))));
    const Expr* rd = k.v(x);
    std::vector<const Stmt*> body = {k.decl(x, nullptr)};
    if (read_before) body.push_back(k.es(rd));
    body.push_back(k.iff(k.gt(k.asgE(x, k.sub(k.pv(f, 0), I(k, "1"))), I(k, "0")), k.blk({k.ret(k.v(x))})));
    body.push_back(k.ret(k.v(x)));
    f->body = k.blk(body);
    k.done(f);
    if (read_before) k.fail("uninitialized", rd->loc);
  };
  lc("fx_if_condition_assign", "int x; if ((x = n - 1) > 0) return x; return x: the condition's write sets the flag on both paths", [=](Kit& k) { cond_write(k, false); });
  lc("fx_if_condition_assign_read_before", "the same with a read of x before the condition", [=](Kit& k) { cond_write(k, true); });

  // while (i++ < n) { c++ } and its twins (S17).
  auto while_incr = [=](Kit& k, int variant) {  // 0 ok, 1 entry, 2 preserved, 3 termination, 4 last-evaluation overflow, 5 postcondition
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    if (variant == 4) k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    else k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq(k.R(f), variant == 5 ? k.add(k.F(f, 0), I(k, "1")) : k.F(f, 0)));
    Var* i = k.local(f, "i", k.i32);
    Var* c = k.local(f, "c", k.i32);
    Clause inv = k.cl(k.land(k.land(k.eq(k.v(i), k.v(c)), k.ge_c(k.v(c), k.i32, "0")), k.le(k.v(c), k.pv(f, 0))));
    const Expr* inc = k.incE(i);
    Clause m = k.cl(variant == 3 ? k.cv(k.i64, k.pv(f, 0)) : k.gap(k.pv(f, 0), k.v(i)));
    const Stmt* body = k.blk({variant == 2 ? k.asg(c, k.add(k.v(c), I(k, "2"))) : plus1(k, c)});
    const Stmt* w = k.loop(k.lt(inc, k.pv(f, 0)), {inv}, {m}, body, nullptr, LF::While, k.jid());
    const Stmt* r = k.ret(k.v(c));
    f->body = k.blk({k.decl(i, I(k, variant == 1 ? "1" : "0")), k.decl(c, I(k, "0")), w, r});
    k.done(f);
    if (variant == 1) k.fail("invariant-entry", inv.loc);
    if (variant == 2) k.fail("invariant-preserved", inv.loc);
    if (variant == 3) k.fail("termination", m.loc);
    if (variant == 4) k.fail("overflow", inc->loc);
    if (variant == 5) k.fail("postcondition", r->loc);
  };
  lc("fx_while_incr_condition", "while (i++ < n) c++ with invariant i == c <= n: returns n", [=](Kit& k) { while_incr(k, 0); });
  lc("fx_while_incr_condition_entry", "i starts at 1: the invariant fails on entry", [=](Kit& k) { while_incr(k, 1); });
  lc("fx_while_incr_condition_preserved", "the body adds 2 to c: the invariant is not preserved", [=](Kit& k) { while_incr(k, 2); });
  lc("fx_while_incr_condition_termination", "a constant measure does not decrease", [=](Kit& k) { while_incr(k, 3); });
  lc("fx_while_incr_condition_last_evaluation", "n up to INT_MAX: the i++ of the last evaluation overflows", [=](Kit& k) { while_incr(k, 4); });
  lc("fx_while_incr_condition_post", "the postcondition r == n + 1 needs the exit state", [=](Kit& k) { while_incr(k, 5); });

  // continue and break with a condition that has an effect (the exit jump and the continue label coexist).
  auto top_jump = [=](Kit& k, bool is_break, bool bad) {
    Function* f = k.fn("f", k.i32, {{"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    Var* i = k.local(f, "i", k.i32);
    Var* c = k.local(f, "c", k.i32);
    const int id = k.jid();
    Clause inv;
    const Expr* post;
    if (!is_break) {
      inv = k.cl(k.land(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 0))),
                        k.lor(k.land(k.le_c(k.v(i), k.i32, "2"), k.eq_c(k.v(c), k.i32, "0")),
                              k.land(k.gt(k.v(i), I(k, bad ? "3" : "2")), k.eq(k.v(c), k.sub(k.v(i), I(k, "2")))))));
      post = k.lor(k.land(k.le(k.F(f, 0), I(k, "2")), k.eq_c(k.R(f), k.i32, "0")),
                   k.land(k.gt(k.F(f, 0), I(k, "2")), k.eq(k.R(f), k.sub(k.F(f, 0), I(k, "2")))));
    } else {
      inv = k.cl(k.land(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 0))), k.land(k.le_c(k.v(i), k.i32, "4"), k.eq(k.v(c), k.v(i)))));
      post = k.lor(k.land(k.lt(k.F(f, 0), I(k, "4")), k.eq(k.R(f), k.F(f, 0))),
                   k.land(k.ge(k.F(f, 0), I(k, "4")), k.eq_c(k.R(f), k.i32, bad ? "5" : "4")));
    }
    k.ens(f, post);
    const Stmt* body = is_break ? k.blk({k.iff(k.eq_c(k.v(i), k.i32, "5"), k.blk({k.brk(id)})), plus1(k, c)})
                                : k.blk({k.iff(k.le_c(k.v(i), k.i32, "2"), k.blk({k.cnt(id)})), plus1(k, c)});
    const Stmt* w = k.loop(k.lt(k.incE(i), k.pv(f, 0)), {inv}, {k.cl(k.gap(k.pv(f, 0), k.v(i)))}, body, nullptr, LF::While, id);
    const Stmt* r = k.ret(k.v(c));
    f->body = k.blk({k.decl(i, I(k, "0")), k.decl(c, I(k, "0")), w, r});
    k.done(f);
    if (bad && !is_break) k.fail("invariant-preserved", inv.loc);
    if (bad && is_break) k.fail("postcondition", r->loc);
  };
  lc("fx_while_incr_condition_continue", "while (i++ < n) { if (i <= 2) continue; c++; }: continue goes back to the condition", [=](Kit& k) { top_jump(k, false, false); });
  lc("fx_while_incr_condition_continue_bad", "the invariant forgets i == 3: not preserved on the continue path", [=](Kit& k) { top_jump(k, false, true); });
  lc("fx_while_incr_condition_break", "while (i++ < n) { if (i == 5) break; c++; }: leaves through the break", [=](Kit& k) { top_jump(k, true, false); });
  lc("fx_while_incr_condition_break_bad", "the postcondition says 5 on the break path", [=](Kit& k) { top_jump(k, true, true); });

  // do { c++ } while (i++ < n): the condition's effect after the body.
  auto do_incr = [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    k.ens(f, k.eq(k.R(f), wrong ? k.F(f, 0) : k.add(k.F(f, 0), I(k, "1"))));
    Var* i = k.local(f, "i", k.i32);
    Var* c = k.local(f, "c", k.i32);
    Clause inv = k.cl(k.land(k.land(k.eq(k.v(i), k.v(c)), k.ge_c(k.v(c), k.i32, "0")), k.le(k.v(c), k.pv(f, 0))));
    const Stmt* w = k.loop(k.lt(k.incE(i), k.pv(f, 0)), {inv}, {k.cl(k.gap(k.pv(f, 0), k.v(c)))}, k.blk({plus1(k, c)}), nullptr, LF::DoWhile, k.jid());
    const Stmt* r = k.ret(k.v(c));
    f->body = k.blk({k.decl(i, I(k, "0")), k.decl(c, I(k, "0")), w, r});
    k.done(f);
    if (wrong) k.fail("postcondition", r->loc);
  };
  lc("fx_do_incr_condition", "do c++; while (i++ < n): c == n + 1", [=](Kit& k) { do_incr(k, false); });
  lc("fx_do_incr_condition_wrong", "the same with r == n", [=](Kit& k) { do_incr(k, true); });

  // for (i = 0, j = n; i < j; i++, j--) c++: the statements of the comma in the init and the step.
  auto for_comma = [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    k.ens(f, k.eq(k.R(f), k.div(wrong ? k.F(f, 0) : k.add(k.F(f, 0), I(k, "1")), I(k, "2"))));
    Var* i = k.local(f, "i", k.i32);
    Var* j = k.local(f, "j", k.i32);
    Var* c = k.local(f, "c", k.i32);
    Clause inv = k.cl(k.land(k.land(k.eq(k.v(i), k.v(c)), k.eq(k.add(k.v(i), k.v(j)), k.pv(f, 0))),
                             k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.add(k.v(j), I(k, "1"))))));
    const Stmt* w = k.loop(k.lt(k.v(i), k.v(j)), {inv}, {k.cl(k.gap(k.v(j), k.v(i)))}, k.blk({plus1(k, c)}), k.blk({k.incS(i), k.decS(j)}), LF::While, k.jid());
    const Stmt* r = k.ret(k.v(c));
    f->body = k.blk({k.decl(i, I(k, "0")), k.decl(j, I(k, "0")), k.decl(c, I(k, "0")), k.asg(i, I(k, "0")), k.asg(j, k.pv(f, 0)), w, r});
    k.done(f);
    if (wrong) k.fail("postcondition", r->loc);
  };
  lc("fx_for_comma_step", "for (i = 0, j = n; i < j; i++, j--) c++: the discarded comma is two statements; c == (n + 1) / 2", [=](Kit& k) { for_comma(k, false); });
  lc("fx_for_comma_step_wrong", "the same with c == n / 2 (wrong for odd n)", [=](Kit& k) { for_comma(k, true); });

  // A local that only the condition writes (decision 0019 with the top-test shape).
  auto cond_only_flag = [=](Kit& k, int variant) {  // 0 ok, 1 read of x before the loop
    Function* f = fn_n(k, k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "0"));
    Var* x = k.local(f, "x", k.i32);
    Var* c = k.local(f, "c", k.i32);
    Clause inv = k.cl(k.land(k.ge_c(k.v(c), k.i32, "0"), k.le(k.v(c), k.pv(f, 0))));
    const Expr* rd = k.v(x);
    const Stmt* w = k.loop(k.gt(k.asgE(x, k.sub(k.pv(f, 0), k.v(c))), I(k, "0")), {inv}, {k.cl(k.gap(k.pv(f, 0), k.v(c)))}, k.blk({plus1(k, c)}),
                           nullptr, LF::While, k.jid());
    std::vector<const Stmt*> top = {k.decl(x, nullptr), k.decl(c, I(k, "0"))};
    if (variant == 1) top.push_back(k.chk(k.eq(rd, rd)));
    top.push_back(w);
    top.push_back(k.ret(k.v(x)));
    f->body = k.blk(top);
    k.done(f);
    if (variant == 1) k.fail_only("uninitialized", rd->loc, 2, 0);  // two reads of x; after the first fails Silicon assumes its flag
  };
  lc("fx_while_assign_condition", "int x; while ((x = n - c) > 0) c++; return x: the condition ran at least once, so x has a value; x == 0 at the end", [=](Kit& k) { cond_only_flag(k, 0); });
  lc("fx_while_assign_condition_read_before", "the same with a read of x before the loop", [=](Kit& k) { cond_only_flag(k, 1); });

  // A local written only under && in the condition: its flag and range cross iterations (S17c).
  auto and_cond = [=](Kit& k, int variant) {  // 0 ok, 1 x not assigned before the loop, 2 the range invariants weakened (a hook)
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* c = k.local(f, "c", k.i32);
    std::vector<Clause> inv = {k.cl(k.land(k.ge_c(k.v(c), k.i32, "0"), k.le(k.v(c), k.pv(f, 0))))};
    if (variant == 0) inv.push_back(k.cl(k.eq(k.v(x), k.v(c))));
    if (variant == 0) k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    const Expr* rd = k.v(x);
    const Stmt* w = k.loop(k.land(k.lt(k.v(c), k.pv(f, 0)), k.gt(k.asgE(x, k.add(k.v(c), I(k, "1"))), I(k, "0"))), inv,
                           {k.cl(k.gap(k.pv(f, 0), k.v(c)))}, k.blk({plus1(k, c)}), nullptr, LF::While, k.jid());
    std::vector<const Stmt*> top = {variant == 1 ? k.decl(x, nullptr) : k.decl(x, I(k, "0")), k.decl(c, I(k, "0")), w};
    top.push_back(k.ret(variant == 1 ? rd : k.v(x)));
    f->body = k.blk(top);
    k.done(f);
    if (variant == 1) k.fail("uninitialized", rd->loc);
    if (variant == 2) {
      k.lower_options.weaken_auto_invariant = true;
      k.fail_only("internal", w->loc, 4, 0);  // x and c, entry and preservation: Silicon stops at the first invariant that fails
    }
  };
  lc("fx_while_and_condition_write", "x = 0; while (c < n && (x = c + 1) > 0) c++; return x: the write under && crosses iterations (x == n)", [=](Kit& k) { and_cond(k, 0); });
  lc("fx_while_and_condition_unassigned", "the same with x unassigned before the loop, which may run zero times: the read after it fails", [=](Kit& k) { and_cond(k, 1); });
  lc("fx_while_and_condition_range_hook", "the automatic range invariants of x and c weakened (a self-test hook): Silicon rejects both", [=](Kit& k) { and_cond(k, 2); });

  // ---- guarded operands with effects (branch lowering) ----
  auto and_assign = [=](Kit& k, int variant) {  // 0 ok, 1 wrong value, 2 read x after (untaken path), 3 the same with n > 0 required
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    if (variant == 3) k.req(f, k.gt(k.F(f, 0), I(k, "0")));
    if (variant <= 1) k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, variant == 1 ? "2" : "1")), I(k, "1"), I(k, "0"))));
    const Expr* rd = k.v(x);
    const Stmt* r = k.ret(variant >= 2 ? rd : k.v(y));
    if (variant == 3) k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    f->body = k.blk({variant >= 2 ? k.decl(x, nullptr) : k.decl(x, I(k, "0")), k.decl(y, I(k, "0")),
                     k.asg(y, k.bi(k.i32, k.land(k.gt(k.pv(f, 0), I(k, "0")), k.gt(k.asgE(x, k.pv(f, 0)), I(k, "1"))))), r});
    k.done(f);
    if (variant == 1) k.fail("postcondition", r->loc);
    if (variant == 2) k.fail("uninitialized", rd->loc);
  };
  lc("fx_and_assign", "y = n > 0 && (x = n) > 1: the write happens only when n > 0; y == (n > 1)", [=](Kit& k) { and_assign(k, 0); });
  lc("fx_and_assign_wrong", "the same with the postcondition n > 2", [=](Kit& k) { and_assign(k, 1); });
  lc("fx_and_assign_flag_untaken", "x declared without a value, read after: the branch may not have run", [=](Kit& k) { and_assign(k, 2); });
  lc("fx_and_assign_flag_taken", "the same with n > 0 required: only the branch that writes x is feasible", [=](Kit& k) { and_assign(k, 3); });
  twin("fx_or_assign", "y = n == 0 || (x = n) > 1: the write happens only when n != 0", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "1")), I(k, "1"), k.cond(k.eq_c(k.F(f, 0), k.i32, "0"), I(k, wrong ? "0" : "1"), I(k, "0")))));
    const Stmt* r = k.ret(k.v(y));
    f->body = k.blk({k.decl(x, I(k, "0")), k.decl(y, I(k, "0")),
                     k.asg(y, k.bi(k.i32, k.lor(k.eq_c(k.pv(f, 0), k.i32, "0"), k.gt(k.asgE(x, k.pv(f, 0)), I(k, "1"))))), r});
    k.done(f);
    return r;
  });
  auto and_div = [=](Kit& k, bool strict) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    const Expr* d = k.div(I(k, "100"), k.asgE(x, k.pv(f, 0)));
    f->body = k.blk({k.decl(x, I(k, "0")), k.ret(k.bi(k.i32, k.land(k.bin(strict ? BinaryOp::Gt : BinaryOp::Ge, k.pv(f, 0), I(k, "0")), k.gt(d, I(k, "1")))))});
    k.done(f);
    if (!strict) k.fail("division-by-zero", d->loc);
  };
  lc("fx_and_assign_division_in_branch", "n > 0 && 100 / (x = n) > 1: the division check is inside the branch, under the guard", [=](Kit& k) { and_div(k, true); });
  lc("fx_and_assign_division_unguarded", "n >= 0 && 100 / (x = n) > 1: n may be 0", [=](Kit& k) { and_div(k, false); });
  twin("fx_nested_branches", "n > 0 && (n > 5 || (x = 1) > 0): a branch inside a branch", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, wrong ? "5" : "0")), I(k, "1"), I(k, "0"))));
    const Stmt* r = k.ret(k.bi(k.i32, k.land(k.gt(k.pv(f, 0), I(k, "0")), k.lor(k.gt(k.pv(f, 0), I(k, "5")), k.gt(k.asgE(x, I(k, "1")), I(k, "0"))))));
    f->body = k.blk({k.decl(x, I(k, "0")), r});
    k.done(f);
    return r;
  });
  auto cond_arms = [=](Kit& k, int variant) {  // 0 ok, 1 wrong value, 2 one arm leaves x unassigned and x is read after, 3 the same without the read
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    const Expr* second = variant >= 2 ? I(k, "0") : k.asgE(x, I(k, "2"));
    if (variant <= 1) k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "5")), I(k, "2"), I(k, variant == 1 ? "5" : "4"))));
    if (variant == 3) k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "5")), I(k, "1"), I(k, "0"))));
    const Expr* rd = k.v(x);
    const Stmt* r = k.ret(variant == 2 ? k.add(rd, k.v(y)) : (variant == 3 ? k.v(y) : k.add(k.v(x), k.v(y))));
    f->body = k.blk({variant == 2 ? k.decl(x, nullptr) : k.decl(x, I(k, "0")), k.decl(y, I(k, "0")),
                     k.asg(y, k.cond(k.gt(k.pv(f, 0), I(k, "5")), k.asgE(x, I(k, "1")), second)), r});
    k.done(f);
    if (variant == 1) k.fail("postcondition", r->loc);
    if (variant == 2) k.fail("uninitialized", rd->loc);
  };
  lc("fx_cond_assign_arms", "y = n > 5 ? (x = 1) : (x = 2); return x + y: the value of an arm is the stored value", [=](Kit& k) { cond_arms(k, 0); });
  lc("fx_cond_assign_arms_wrong", "the same with 5 instead of 4 for the else arm", [=](Kit& k) { cond_arms(k, 1); });
  lc("fx_cond_assign_one_arm_flag", "only the first arm writes x, and x is read after", [=](Kit& k) { cond_arms(k, 2); });
  lc("fx_cond_assign_one_arm_unread", "the same without the read of x", [=](Kit& k) { cond_arms(k, 3); });
  twin("fx_cond_incdec_arms", "y = n > 5 ? x++ : x--: either arm; y + x == 2n + 1 or 2n - 1", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "5")), k.add(k.mul(I(k, "2"), k.F(f, 0)), I(k, "1")),
                                 k.sub(k.mul(I(k, "2"), k.F(f, 0)), I(k, wrong ? "2" : "1")))));
    const Stmt* r = k.ret(k.add(k.v(y), k.v(x)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, I(k, "0")), k.asg(y, k.cond(k.gt(k.pv(f, 0), I(k, "5")), k.incE(x), k.decE(x))), r});
    k.done(f);
    return r;
  });

  // ---- read snapshots (E3) ----
  auto snapshot = [=](Kit& k, int variant) {  // 0 ok, 1 overflow of y + x, 2 x unassigned at the first read, 3 read after the write only (ok), 4 wrong value
    Function* f = k.fn("f", k.i32, {{"p", k.i32}});
    if (variant == 0 || variant == 4) k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    if (variant == 0) k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), I(k, "1"))));
    if (variant == 4) k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    Var* r = k.local(f, "r", k.i32);
    const Expr* rd = k.v(x);
    const Expr* sum = k.add(k.v(y), k.v(x));
    const Expr* value = variant == 3 ? k.comma(k.asgE(x, I(k, "5")), k.add(k.v(x), I(k, "1")))
                                     : k.comma(k.comma(k.asgE(y, rd), k.asgE(x, I(k, "1"))), sum);
    const Stmt* ret = k.ret(k.v(r));
    f->body = k.blk({variant == 2 ? k.decl(x, nullptr) : k.decl(x, k.pv(f, 0)), k.decl(y, I(k, "0")), k.decl(r, I(k, "0")), k.asg(r, value), ret});
    k.done(f);
    if (variant == 1) k.fail("overflow", sum->loc);
    if (variant == 2) k.fail("uninitialized", rd->loc);
    if (variant == 4) k.fail("postcondition", ret->loc);
  };
  lc("fx_snapshot_old_value", "r = (y = x, x = 1, y + x): y is the old x, the second read of x sees 1: r == p + 1", [=](Kit& k) { snapshot(k, 0); });
  lc("fx_snapshot_overflow", "the same for any p: y + x overflows at INT_MAX", [=](Kit& k) { snapshot(k, 1); });
  lc("fx_snapshot_uninitialized", "the same with x unassigned: the first read fails", [=](Kit& k) { snapshot(k, 2); });
  lc("fx_snapshot_wrong_value", "the same with the postcondition r == p", [=](Kit& k) { snapshot(k, 4); });
  lc("fx_snapshot_after_write", "r = (x = 5, x + 1) is 6: the read after the write sees the new value", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"p", k.i32}});
    k.ens(f, k.eq_c(k.R(f), k.i32, "6"));
    Var* x = k.local(f, "x", k.i32);
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(k.comma(k.asgE(x, I(k, "5")), k.add(k.v(x), I(k, "1"))))});
    k.done(f);
  });
  lc("fx_snapshot_after_write_wrong", "the same with the postcondition r == p + 1: the old value is gone", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"p", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), I(k, "1"))));
    Var* x = k.local(f, "x", k.i32);
    const Stmt* r = k.ret(k.comma(k.asgE(x, I(k, "5")), k.add(k.v(x), I(k, "1"))));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    k.fail("postcondition", r->loc);
  });

  // ---- shapes the compilers accept silently (P35): well-defined, so accepted and lowered; each has a twin that pins its value ----
  twin("fx_seq_store_reads_target", "return x = x + 1: a read of the target in the stored value is no conflict", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), I(k, wrong ? "2" : "1"))));
    const Stmt* r = k.ret(k.asgE(x, k.add(k.v(x), I(k, "1"))));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_comma_before_read", "return (x = 1, x) + 1: the comma sequences the write before the read", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "3" : "2"));
    const Stmt* r = k.ret(k.add(k.comma(k.asgE(x, I(k, "1")), k.v(x)), I(k, "1")));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_and_reads_after_write", "return x++ != 0 && x > 0: && sequences the increment before the read", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.cond(k.ne(k.F(f, 0), I(k, "0")), I(k, wrong ? "0" : "1"), I(k, wrong ? "1" : "0"))));
    const Stmt* r = k.ret(k.bi(k.i32, k.land(k.ne(k.incE(x), I(k, "0")), k.gt(k.v(x), I(k, "0")))));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_comma_incr_then_read", "return (x++, x): the value is n + 1", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.F(f, 0), I(k, wrong ? "2" : "1"))));
    const Stmt* r = k.ret(k.comma(k.incE(x), k.v(x)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_store_comma_shields", "x = (x++, 2): the comma sequences the increment before the store: x == 2", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "3" : "2"));
    const Stmt* r = k.ret(k.v(x));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.asg(x, k.comma(k.incE(x), I(k, "2"))), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_store_call_shields", "return x = g(x++): the call sequences the increment before the store", [=](Kit& k, bool wrong) {
    Function* g = g_id(k, false);
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), wrong ? k.add(k.F(f, 0), I(k, "1")) : k.F(f, 0)));
    const Stmt* r = k.ret(k.asgE(x, k.call(g, {k.incE(x)})));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_store_and_first_operand_shields", "x = (x++ != 0 && 1 > 0): the first operand of && is sequenced before the store", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.cond(k.ne(k.F(f, 0), I(k, "0")), I(k, wrong ? "0" : "1"), I(k, wrong ? "1" : "0"))));
    const Stmt* r = k.ret(k.v(x));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.asg(x, k.bi(k.i32, k.land(k.ne(k.incE(x), I(k, "0")), k.gt(I(k, "1"), I(k, "0"))))), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_decl_of_assign_value", "int y = x = x + 1: y and x are both n + 1", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq(k.R(f), k.add(k.mul(I(k, "2"), k.F(f, 0)), I(k, wrong ? "3" : "2"))));
    const Stmt* r = k.ret(k.add(k.v(x), k.v(y)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, k.asgE(x, k.add(k.v(x), I(k, "1")))), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_comma_then_postincr", "y = (x = 2, x++): y == 2 and x == 3", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "6" : "5"));
    const Stmt* r = k.ret(k.add(k.v(x), k.v(y)));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, k.comma(k.asgE(x, I(k, "2")), k.incE(x))), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_cond_condition_then_arm_read", "return (x = 1) != 0 ? x : 0: the condition is sequenced before the arms", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "0" : "1"));
    const Stmt* r = k.ret(k.cond(k.ne(k.asgE(x, I(k, "1")), I(k, "0")), k.v(x), I(k, "0")));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_different_variables", "return (x = 1) + (y = 2): two writes of different variables are unsequenced but independent", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "4" : "3"));
    const Stmt* r = k.ret(k.add(k.asgE(x, I(k, "1")), k.asgE(y, I(k, "2"))));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.decl(y, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });
  twin("fx_seq_cond_arms_not_checked_against_each_other", "return n > 3 ? x++ : (x = 2): at most one arm runs", [=](Kit& k, bool wrong) {
    Function* f = fn_n(k, k.i32);
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.eq(k.R(f), k.cond(k.gt(k.F(f, 0), I(k, "3")), wrong ? k.add(k.F(f, 0), I(k, "1")) : k.F(f, 0), I(k, "2"))));
    const Stmt* r = k.ret(k.cond(k.gt(k.pv(f, 0), I(k, "3")), k.incE(x), k.asgE(x, I(k, "2"))));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), r});
    k.done(f);
    return r;
  });

  // ---- a callee that does not return hides only what depends on it (decision 0012 item 6, 0020 item 7) ----
  // `nr(x)`: requires x >= 0, ensures false, a trusted contract.  Whatever C may evaluate before the call is checked before it, so the
  // checks of the operand that does not depend on the call still fail; a check that depends on the call, or that C sequences after it,
  // is dead after it and holds.
  auto never_returns = [=](Kit& k) {
    Function* nr = k.fn("nr", k.i32, {{"x", k.i32}});
    k.req(nr, k.ge_c(k.F(nr, 0), k.i32, "0"));
    k.ens(nr, k.no());
    k.trusted(nr);
    return nr;
  };
  lc("fx_noreturn_unsequenced_write_checks", "return nr(0) + 100 / y++ for any y: y == 0, and y++ at INT_MAX, are errors C may reach before the call", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32, {{"y0", k.i32}});
    Var* y = k.local(f, "y", k.i32);
    const Expr* inc = k.incE(y);
    const Expr* d = k.div(I(k, "100"), inc);
    f->body = k.blk({k.decl(y, k.pv(f, 0)), k.ret(k.add(k.call(nr, {I(k, "0")}), d))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
    k.fail("overflow", inc->loc);
  });
  lc("fx_noreturn_unsequenced_write_bounded", "the same with y in [1, 100]: nothing before the call can fail, and the rest depends on the call", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32, {{"y0", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "1", "100"));
    Var* y = k.local(f, "y", k.i32);
    f->body = k.blk({k.decl(y, k.pv(f, 0)), k.ret(k.add(k.call(nr, {I(k, "0")}), k.div(I(k, "100"), k.incE(y))))});
    k.done(f);
  });
  lc("fx_noreturn_unsequenced_uninitialized", "int y; return nr(0) + y++: the read of y comes before the call in some order", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32);
    Var* y = k.local(f, "y", k.i32);
    const Expr* inc = k.incE(y);
    f->body = k.blk({k.decl(y, nullptr), k.ret(k.add(k.call(nr, {I(k, "0")}), inc))});
    k.done(f);
    k.fail("uninitialized", inc->loc);
  });
  lc("fx_noreturn_comma_left_effects_do_not_floor", "return nr(0) + (y++, 100 / d): the comma's left operand does not depend on the call, so neither does the check", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32, {{"y0", k.i32}, {"d", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "100"));
    Var* y = k.local(f, "y", k.i32);
    const Expr* d = k.div(I(k, "100"), k.pv(f, 1));
    f->body = k.blk({k.decl(y, k.pv(f, 0)), k.ret(k.add(k.call(nr, {I(k, "0")}), k.comma(k.incE(y), d)))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("fx_noreturn_branch_checks_before_call", "return nr(0) + (c && (x = 100 / d) > 1): the branch does not depend on the call; the division inside it is checked first", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32, {{"c", k.bl}, {"d", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    const Expr* div = k.div(I(k, "100"), k.pv(f, 1));
    f->body = k.blk({k.decl(x, I(k, "0")),
                     k.ret(k.add(k.call(nr, {I(k, "0")}), k.bi(k.i32, k.land(k.pv(f, 0), k.gt(k.asgE(x, div), I(k, "1"))))))});
    k.done(f);
    k.fail("division-by-zero", div->loc);
  });
  lc("fx_noreturn_argument_chain", "h((nr0(), 1), y++) with a callee h: the write of y and its checks come before the calls", [=](Kit& k) {
    Function* nr0 = k.fn("nr0", k.vd);
    k.ens(nr0, k.no());
    k.trusted(nr0);
    Function* h = k.fn("h", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.trusted(h);
    Function* f = k.fn("f", k.i32, {{"y0", k.i32}});
    Var* y = k.local(f, "y", k.i32);
    const Expr* inc = k.incE(y);
    f->body = k.blk({k.decl(y, k.pv(f, 0)), k.ret(k.call(h, {k.comma(k.call(nr0, {}), I(k, "1")), inc}))});
    k.done(f);
    k.fail("overflow", inc->loc);
  });
  lc("fx_noreturn_dependent_check_is_dead", "return 100 / nr(nr(0)): every check depends on a call, so none can fail", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32);
    f->body = k.blk({k.ret(k.div(I(k, "100"), k.call(nr, {k.call(nr, {I(k, "0")})})))});
    k.done(f);
  });
  lc("fx_noreturn_comma_sequences_check_after_call", "return (nr(0), 100 / d): the comma sequences the division after the call: holds for any d", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32, {{"d", k.i32}});
    f->body = k.blk({k.ret(k.comma(k.call(nr, {I(k, "0")}), k.div(I(k, "100"), k.pv(f, 0))))});
    k.done(f);
  });
  lc("fx_noreturn_comma_sequences_write_after_call", "return (nr(0), y++ / 1) with any y: the increment is sequenced after the call, so its overflow cannot fail", [=](Kit& k) {
    Function* nr = never_returns(k);
    Function* f = k.fn("f", k.i32, {{"y0", k.i32}});
    Var* y = k.local(f, "y", k.i32);
    f->body = k.blk({k.decl(y, k.pv(f, 0)), k.ret(k.comma(k.call(nr, {I(k, "0")}), k.incE(y)))});
    k.done(f);
  });
}

// ---- rejections of the new nodes and of unsequenced modifications ------------------------------------------------------

void define_side_effect_rejections() {
  const char* kUnseq = "unsequenced modification";
  // A function f(c: bool, a: int) with locals x and y (both int, x = y = a); `body_of` builds the statement of the shape.
  auto shape = [](Kit& k, const std::function<const Stmt*(Kit&, Function*, Var*, Var*)>& body_of) {
    Function* f = k.fn("f", k.vd, {{"c", k.bl}, {"a", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    Var* y = k.local(f, "y", k.i32);
    f->body = k.blk({k.decl(x, k.pv(f, 1)), k.decl(y, k.pv(f, 1)), body_of(k, f, x, y)});
    k.done(f);
  };
  auto expr_shape = [shape](Kit& k, const std::function<const Expr*(Kit&, Function*, Var*, Var*)>& e) {
    shape(k, [e](Kit& k2, Function* f, Var* x, Var* y) { return k2.es(e(k2, f, x, y)); });
  };
  auto I = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto g1 = [](Kit& k) {
    Function* g = k.fn("g", k.i32, {{"p", k.i32}});
    k.trusted(g);
    return g;
  };
  auto g2 = [](Kit& k) {
    Function* g = k.fn("g2", k.i32, {{"p", k.i32}, {"q", k.i32}});
    k.trusted(g);
    return g;
  };

  rc("fx_unseq_i_incr_plus_i", kUnseq, [=](Kit& k) { expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.incE(x), k2.v(x)); }); });
  rc("fx_unseq_read_plus_incr", kUnseq, [=](Kit& k) { expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.v(x), k2.incE(x)); }); });
  rc("fx_unseq_store_postincr", kUnseq, [=](Kit& k) { shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.incE(x)); }); });
  rc("fx_unseq_store_preincr", kUnseq, [=](Kit& k) { shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.incE(x, false)); }); });
  rc("fx_unseq_store_assign", kUnseq, [=](Kit& k) { shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.asgE(x, I(k2, "1"))); }); });
  rc("fx_unseq_store_compound", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.cmpE(x, BinaryOp::Add, I(k2, "1"))); });
  });
  rc("fx_unseq_store_incr_plus_one", kUnseq, [=](Kit& k) { shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.add(k2.incE(x), I(k2, "1"))); }); });
  rc("fx_unseq_store_cond_arm", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function* f, Var* x, Var*) { return k2.asg(x, k2.cond(k2.pv(f, 0), k2.incE(x), I(k2, "0"))); });
  });
  rc("fx_unseq_store_comma_right", kUnseq, [=](Kit& k) { shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.comma(I(k2, "1"), k2.incE(x))); }); });
  rc("fx_unseq_store_and_right", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function* f, Var* x, Var*) { return k2.asg(x, k2.bi(k2.i32, k2.land(k2.pv(f, 0), k2.tb(k2.incE(x))))); });
  });
  rc("fx_unseq_store_plus_assign", kUnseq, [=](Kit& k) { shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asg(x, k2.add(k2.v(x), k2.asgE(x, I(k2, "1")))); }); });
  rc("fx_unseq_compound_incr", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.cmpE(x, BinaryOp::Add, k2.incE(x)); });
  });
  rc("fx_unseq_compound_assign", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.cmpE(x, BinaryOp::Add, k2.asgE(x, I(k2, "1"))); });
  });
  rc("fx_unseq_call_arguments", kUnseq, [=](Kit& k) {
    Function* g = g2(k);
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.call(g, {k2.incE(x), k2.v(x)}); });
  });
  rc("fx_unseq_assign_plus_read", kUnseq, [=](Kit& k) { expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.asgE(x, I(k2, "1")), k2.v(x)); }); });
  rc("fx_unseq_two_assigns", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.asgE(x, I(k2, "1")), k2.asgE(x, I(k2, "2"))); });
  });
  rc("fx_unseq_two_assigns_product", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function* f, Var* x, Var*) { return k2.mul(k2.asgE(x, k2.pv(f, 1)), k2.asgE(x, k2.pv(f, 1))); });
  });
  rc("fx_unseq_cond_arm_plus_read", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function* f, Var* x, Var*) { return k2.add(k2.cond(k2.pv(f, 0), k2.incE(x), I(k2, "0")), k2.v(x)); });
  });
  rc("fx_unseq_comma_plus_read", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.comma(k2.incE(x), I(k2, "0")), k2.v(x)); });
  });
  rc("fx_unseq_comma_chain_plus_read", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.comma(k2.comma(k2.incE(x), I(k2, "0")), I(k2, "3")), k2.v(x)); });
  });
  rc("fx_unseq_two_incr", kUnseq, [=](Kit& k) { expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.incE(x), k2.incE(x)); }); });
  rc("fx_unseq_incr_times_preincr", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.mul(k2.incE(x), k2.incE(x, false)); });
  });
  rc("fx_unseq_incr_plus_read_in_decl", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function* f, Var* x, Var*) {
      Var* z = k2.local(f, "z", k2.i32);
      return k2.decl(z, k2.add(k2.incE(x), k2.v(x)));
    });
  });
  rc("fx_unseq_call_result_plus_read", kUnseq, [=](Kit& k) {
    Function* g = g1(k);
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.call(g, {k2.incE(x)}), k2.v(x)); });
  });
  rc("fx_unseq_comparison_plus_read", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.bi(k2.i32, k2.lt(k2.incE(x), I(k2, "3"))), k2.v(x)); });
  });
  rc("fx_unseq_store_other_target", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var* y) { return k2.asg(y, k2.add(k2.incE(x), k2.asgE(x, I(k2, "1")))); });
  });
  rc("fx_unseq_comma_pair_in_sum", kUnseq, [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.add(k2.comma(k2.incE(x), I(k2, "0")), k2.comma(k2.incE(x), I(k2, "0")));
    });
  });
  rc("fx_unseq_calls_with_assigns", kUnseq, [=](Kit& k) {
    Function* g = g1(k);
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.add(k2.call(g, {k2.asgE(x, I(k2, "1"))}), k2.call(g, {k2.asgE(x, I(k2, "2"))})); });
  });
  rc("fx_unseq_in_loop_condition", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.lt(k2.add(k2.incE(x), k2.v(x)), I(k2, "5")), {}, {k2.cl(I(k2, "0"))}, k2.blk({}), nullptr, LoopForm::While, k2.jid());
    });
  });
  rc("fx_unseq_in_if_condition", kUnseq, [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.iff(k2.lt(k2.add(k2.incE(x), k2.v(x)), I(k2, "5")), k2.blk({})); });
  });
  rc("fx_unseq_in_return", kUnseq, [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(k.add(k.incE(x), k.v(x)))});
    k.done(f);
  });

  // ---- the typing and context rules of the new nodes ----
  rc("fx_rej_effect_in_check", "side effect", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.chk(k2.ne(k2.asgE(x, I(k2, "1")), I(k2, "0"))); });
  });
  rc("fx_rej_effect_in_assert", "side effect", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.asrt(k2.ne(k2.incE(x), I(k2, "0"))); });
  });
  rc("fx_rej_effect_in_invariant", "side effect", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.lt(k2.v(x), I(k2, "5")), {k2.cl(k2.ne(k2.incE(x), I(k2, "7")))}, {k2.cl(I(k2, "0"))}, k2.blk({}), nullptr, LoopForm::While, 0);
    });
  });
  rc("fx_rej_effect_in_measure", "side effect", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.lt(k2.v(x), I(k2, "5")), {}, {k2.cl(k2.incE(x))}, k2.blk({}), nullptr, LoopForm::While, 0);
    });
  });
  rc("fx_rej_effect_in_ensures", "side effect", [=](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    Var* x = k.local(f, "x", k.i32);
    k.ens(f, k.ne(k.asgE(x, I(k, "1")), I(k, "0")));
    f->body = k.blk({k.decl(x, k.pv(f, 0)), k.ret(I(k, "0"))});
    k.done(f);
  });
  rc("fx_rej_effect_in_case_value", "side effect", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function* f, Var* x, Var*) {
      const int id = k2.jid();
      return k2.sw(k2.pv(f, 1), k2.blk({k2.cs(k2.asgE(x, I(k2, "1")), "1")}), id);
    });
  });
  rc("fx_rej_effect_in_do_once_condition", "side effect", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.ne(k2.asgE(x, I(k2, "0")), I(k2, "0")), {}, {}, k2.blk({}), nullptr, LoopForm::DoOnce, k2.jid());
    });
  });
  rc("fx_rej_comma_in_check", "comma operator", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var*, Var*) { return k2.chk(k2.ne(k2.comma(I(k2, "1"), I(k2, "2")), I(k2, "0"))); });
  });
  rc("fx_rej_node_reachable_twice", "reachable twice", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      const Expr* e = k2.asgE(x, I(k2, "1"));
      return k2.blk({k2.es(e), k2.es(e)});
    });
  });
  rc("fx_rej_node_shared_in_expression", "reachable twice", [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      const Expr* e = k2.incE(x);
      return k2.add(e, e);
    });
  });
  rc("fx_rej_call_reachable_twice", "call that is reachable twice", [=](Kit& k) {
    Function* g = g1(k);
    shape(k, [=](Kit& k2, Function*, Var*, Var*) {
      const Expr* c = k2.call(g, {I(k2, "1")});
      return k2.blk({k2.es(c), k2.es(c)});
    });
  });
  rc("fx_rej_comma_reachable_twice", "comma that is reachable twice", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      const Expr* c = k2.comma(k2.incE(x), I(k2, "1"));
      return k2.blk({k2.es(c), k2.es(c)});
    });
  });
  rc("fx_rej_old_value_not_own_variable", "not a read of its own variable", [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var* y) {
      Expr* e = const_cast<Expr*>(k2.incE(x));
      e->b = k2.v(y);
      return e;
    });
  });
  rc("fx_rej_old_value_not_in_stored_value", "not the node its stored value reads", [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      Expr* e = const_cast<Expr*>(k2.incE(x));
      e->b = k2.v(x);
      return e;
    });
  });
  rc("fx_rej_yields_old_without_old_value", "yields the old value without one", [=](Kit& k) {
    expr_shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      Expr* e = const_cast<Expr*>(k2.asgE(x, I(k2, "1")));
      e->yields_old = true;
      return e;
    });
  });
  rc("fx_rej_assign_to_variable_out_of_scope", "not declared here", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    Var* z = k.local(f, "z", k.i32);
    f->body = k.blk({k.blk({k.decl(z, k.pv(f, 0))}), k.es(k.asgE(z, I(k, "1")))});
    k.done(f);
  });
  rc("fx_rej_assign_wrong_value_type", "where int is required", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) { return k2.es(k2.asgE(x, k2.num(k2.u32, "1"))); });
  });
  rc("fx_rej_comma_void", "comma of type void", [=](Kit& k) {
    Function* v = k.fn("v", k.vd);
    k.trusted(v);
    shape(k, [=](Kit& k2, Function*, Var*, Var*) { return k2.es(k2.comma(I(k2, "1"), k2.call(v, {}))); });
  });
  rc("fx_rej_loop_effect_condition_without_jump_id", "no jump id", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.lt(k2.incE(x), I(k2, "5")), {}, {k2.cl(I(k2, "0"))}, k2.blk({}), nullptr, LoopForm::While, 0);
    });
  });
  rc("fx_rej_call_in_effectful_loop_condition", "a call in a loop condition", [=](Kit& k) {
    Function* g = g1(k);
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.lt(k2.call(g, {k2.incE(x)}), I(k2, "5")), {}, {k2.cl(I(k2, "0"))}, k2.blk({}), nullptr, LoopForm::While, k2.jid());
    });
  });
  rc("fx_rej_effect_with_call_in_guarded_operand", "a call in a conditionally evaluated operand", [=](Kit& k) {
    Function* g = g1(k);
    shape(k, [=](Kit& k2, Function* f, Var* x, Var*) {
      return k2.es(k2.bi(k2.i32, k2.land(k2.pv(f, 0), k2.gt(k2.call(g, {k2.incE(x)}), I(k2, "0")))));
    });
  });
  rc("fx_rej_calls_around_a_comma_in_a_sum", "not nested in each other", [=](Kit& k) {
    Function* g = g1(k);
    expr_shape(k, [=](Kit& k2, Function*, Var*, Var*) { return k2.add(k2.comma(k2.call(g, {I(k2, "1")}), I(k2, "0")), k2.call(g, {I(k2, "2")})); });
  });
  rc("fx_rej_two_unnested_calls_still", "not nested in each other", [=](Kit& k) {
    Function* g = g1(k);
    expr_shape(k, [=](Kit& k2, Function*, Var*, Var*) { return k2.add(k2.call(g, {I(k2, "1")}), k2.call(g, {I(k2, "2")})); });
  });
  rc("fx_rej_step_is_not_a_statement_list", "not an assignment or an expression statement", [=](Kit& k) {
    shape(k, [=](Kit& k2, Function*, Var* x, Var*) {
      return k2.loop(k2.lt(k2.v(x), I(k2, "5")), {}, {k2.cl(I(k2, "0"))}, k2.blk({}), k2.blk({k2.iff(k2.yes(), k2.blk({}))}), LoopForm::While, 0);
    });
  });
}

}  // namespace
