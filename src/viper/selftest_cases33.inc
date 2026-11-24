// (included by selftest.cpp) Pure functions, M4.5e1d Part B (record 0062 items 5-11): a read `pure` function is the Viper function `pf_<name>__<id>` with its `requires` lines (the parameters' ranges,
// each pointer's shape, the clauses, an atom as its pure line and its `acc(.., wildcard)` and element-range lines), its `ensures` (a user row `postcondition` at the function's `return`), its measure
// and its body in the contract form of its own `requires` sites; its definition check is the method `pf_<name>__<id>__wf` (the body is one more clause of the entry state); an application is `(G ?
// pf_g(args) : d)` in a body, a ghost `hva_<m>__<g>_pre(args)` with facts (i) and (ii) in the `__wf`, and rows (the framing a `permission`, a value clause a `precondition`) before the fact in a `check`.
namespace {

void define_pure_functions()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.b.text(k.num(k.i32, v), v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto RR = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    // a `pure` function `ret name(params) { return <body>; }`
    auto pure_fn = [](Kit& k, const char* name, Type ret, const std::vector<Kit::P>& params,
                      const std::function<const Expr*(Function*)>& body)
    {
        Function* f {k.fn(name, ret, params)};
        f->pure = true;
        f->pure_body = body(f);
        k.done(f);
        return f;
    };
    auto app = [](Kit& k, const Function* f, std::vector<const Expr*> args, const char* text)
    {
        return k.b.apply(f, std::move(args), text, k.at());
    };
    auto arr = [](Kit& k, Function* f, const char* name, Type element, unsigned long long count)
    {
        Var* x {f->new_local(name, k.prog.array_of(element, count), k.at())};
        f->mark_address_taken(x);
        return x;
    };
    auto aat = [](Kit& k, const Var* a, const Expr* i)
    {
        return k.b.elem_addr(k.addr(a), i, k.at());
    };
    auto awr = [=](Kit& k, const Var* a, const Expr* i, const Expr* v)
    {
        return k.store(aat(k, a, i), v);
    };
    // a positive case and its vacuity twin: the same program with `check(false)` last, which must fail
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity", "the same with check(false) after everything: it fails (what the program assumes is not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& st)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    // bool is_small(int x) { return -100 < x && x < 100; }
    auto mk_small = [=](Kit& k)
    {
        return pure_fn(k, "is_small", k.bl, {{"x", k.i32}},
                       [&](Function* f)
                       {
                           return k.land(k.lt(k.num(k.i32, "-100"), k.F(f, 0)), k.lt(k.F(f, 0), k.num(k.i32, "100")));
                       });
    };
    // ---- a function without a contract, in a `check` ----
    lcv("pure_scalar_check", "check(is_small(5)): the function's body, the fact", [=](Kit& k)
        {
            Function* g {mk_small(k)};
            Function* f {k.fn("use", k.vd)};
            std::vector<const Stmt*> st {k.chk(app(k, g, {k.num(k.i32, "5")}, "is_small(5)"))};
            tail(k, st);
            f->body = k.blk(st);
            k.done(f);
        });
    lc("pure_scalar_check_false", "check(is_small(500)): fails at the fact (the default is not observable, and the application is the body's value)",
       [=](Kit& k)
       {
           Function* g {mk_small(k)};
           Function* f {k.fn("use", k.vd)};
           const Stmt* c {k.chk(app(k, g, {k.num(k.i32, "500")}, "is_small(500)"))};
           f->body = k.blk({c});
           k.done(f);
           k.fail("check", c->loc);
       });
    // long sum_to(const int *a, int n) requires(0 <= n && reads_range(a, n)) ensures(r, -2147483648LL * n <= r && r <= 2147483647LL * n) decreases(n)
    //     { return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1]; }
    // `variant`: 0 the function, 1 no `decreases` (the recursion needs one), 2 the self-call with `n`, 3 the body adds `a[n]` (one past the atom's range), 4 the wrong bound in the `ensures`
    auto mk_sum = [=](Kit& k, int variant)
    {
        Function* f {k.fn("sum_to", k.i64, {{"a", PI(k)}, {"n", k.i32}})};
        f->pure = true;
        k.req(f, k.land(k.ge_c(fx(k, f, 1), k.i32, "0"), RR(k, f, 0, 1, 0)));
        const Expr* lo {k.mul(k.num(k.i64, "-2147483648"), k.cv(k.i64, fx(k, f, 1)))};
        const Expr* hi {k.mul(k.num(k.i64, variant == 4 ? "2147483646" : "2147483647"), k.cv(k.i64, fx(k, f, 1)))};
        k.ens(f, k.land(k.le(lo, k.R(f)), k.le(k.R(f), hi)));
        if (variant != 1)
        {
            k.dec(f, fx(k, f, 1));
        }
        const Expr* nm1 {k.sub(k.F(f, 1), k.num(k.i32, "1"))};
        const Expr* self {app(k, f, {k.F(f, 0), variant == 2 ? k.F(f, 1) : nm1}, "sum_to(a, n - 1)")};
        const Expr* elem {k.deref(k.b.elem_addr(k.F(f, 0), variant == 3 ? k.F(f, 1) : k.sub(k.F(f, 1), k.num(k.i32, "1")), k.at()))};
        f->pure_body = k.cond(k.eq(k.F(f, 1), k.num(k.i32, "0")), k.num(k.i64, "0"), k.add(self, k.cv(k.i64, elem)));
        k.done(f);
        return f;
    };
    lcv("pure_sum_definition", "the recursive sum with its measure and bound: the function, its `__wf` (the guarded self-application, facts (i) and (ii)) verify", [=](Kit& k)
        {
            Function* g {mk_sum(k, 0)};
            Function* f {k.fn("use", k.vd)};
            std::vector<const Stmt*> st;
            tail(k, st);
            f->body = k.blk(st);
            k.done(f);
            (void)g;
        });
    // ---- the definition check (item 9): each obligation of the body is a row in the function, at the operation ----
    // int twice(int x) [requires(-1000 <= x && x <= 1000)] { return x + x; }: the sum may overflow without the bound
    auto mk_twice = [=](Kit& k, bool bounded, const Expr** sum)
    {
        Function* f {k.fn("twice", k.i32, {{"x", k.i32}})};
        f->pure = true;
        if (bounded)
        {
            k.req(f, k.range(k.F(f, 0), k.i32, "-1000", "1000"));
        }
        *sum = k.add(k.F(f, 0), k.F(f, 0));
        f->pure_body = *sum;
        k.done(f);
        return f;
    };
    lc("pure_overflow_bounded", "x + x under -1000 <= x <= 1000: the function and its `__wf` verify", [=](Kit& k)
       {
           const Expr* sum {nullptr};
           mk_twice(k, true, &sum);
       });
    lc("pure_overflow_unbounded", "x + x with no bound: the overflow row of the `__wf`, in the function, at the sum", [=](Kit& k)
       {
           const Expr* sum {nullptr};
           mk_twice(k, false, &sum);
           k.fail("overflow", sum->loc);
       });
    // short narrow(int x) [requires(-1000 <= x && x <= 1000)] { return (short)x; }
    auto mk_narrow = [=](Kit& k, bool bounded, const Expr** cvt)
    {
        Function* f {k.fn("narrow", k.i16, {{"x", k.i32}})};
        f->pure = true;
        if (bounded)
        {
            k.req(f, k.range(k.F(f, 0), k.i32, "-1000", "1000"));
        }
        *cvt = k.cv(k.i16, k.F(f, 0));
        f->pure_body = *cvt;
        k.done(f);
        return f;
    };
    lc("pure_conversion_bounded", "the conversion at the return of an int to short under a bound: verifies", [=](Kit& k)
       {
           const Expr* c {nullptr};
           mk_narrow(k, true, &c);
       });
    lc("pure_conversion_unbounded", "the conversion at the return without a bound: the `conversion` row in the function", [=](Kit& k)
       {
           const Expr* c {nullptr};
           mk_narrow(k, false, &c);
           k.fail("conversion", c->loc);
       });
    // bool pos(int x) requires(0 < x) { return x < 100; }, checked at 5 and at 0
    auto mk_pos = [=](Kit& k)
    {
        Function* f {k.fn("pos", k.bl, {{"x", k.i32}})};
        f->pure = true;
        k.req(f, k.lt(k.num(k.i32, "0"), k.F(f, 0)));
        f->pure_body = k.lt(k.F(f, 0), k.num(k.i32, "100"));
        k.done(f);
        return f;
    };
    lcv("pure_value_requires_check", "check(pos(5)): the value clause is a precondition row, held", [=](Kit& k)
        {
            Function* g {mk_pos(k)};
            Function* f {k.fn("use", k.vd)};
            std::vector<const Stmt*> st {k.chk(app(k, g, {k.num(k.i32, "5")}, "pos(5)"))};
            tail(k, st);
            f->body = k.blk(st);
            k.done(f);
        });
    lc("pure_value_requires_check_outside", "check(pos(0)): the application is outside the function's `requires`: the `precondition` row, with the note at the clause",
       [=](Kit& k)
       {
           Function* g {mk_pos(k)};
           Function* f {k.fn("use", k.vd)};
           const Expr* a {app(k, g, {k.num(k.i32, "0")}, "pos(0)")};
           const Stmt* c {k.chk(a)};
           f->body = k.blk({c});
           k.done(f);
           k.fail("precondition", a->loc);
       });
    // bool down(int n) requires(0 <= n) decreases(n) { return n == 0 || down(n - 1); } ; `variant` 1: the self-call with n; 2: the measure n - 5
    auto mk_down = [=](Kit& k, int variant, const Expr** self_out)
    {
        Function* f {k.fn("down", k.bl, {{"n", k.i32}})};
        f->pure = true;
        k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
        k.dec(f, variant == 2 ? k.sub(k.F(f, 0), k.num(k.i32, "5")) : k.F(f, 0));
        const Expr* self {app(k, f, {variant == 1 ? k.F(f, 0) : k.sub(k.F(f, 0), k.num(k.i32, "1"))}, "down(n - 1)")};
        *self_out = self;
        f->pure_body = k.lor(k.eq(k.F(f, 0), k.num(k.i32, "0")), self);
        k.done(f);
        return f;
    };
    lc("pure_recursion_down", "a self-recursive bool function with decreases(n): its Viper function (termination) and its `__wf` verify", [=](Kit& k)
       {
           const Expr* self {nullptr};
           mk_down(k, 0, &self);
       });
    lc("pure_recursion_not_decreasing", "the self-call with n: `termination` at the self-call, one error", [=](Kit& k)
       {
           const Expr* self {nullptr};
           mk_down(k, 1, &self);
           k.fail("termination", self->loc);
       });
    lc("pure_recursion_unbounded_measure", "the measure n - 5 may be negative: `termination` at the self-call", [=](Kit& k)
       {
           const Expr* self {nullptr};
           mk_down(k, 2, &self);
           k.fail("termination", self->loc);
       });
    // int tri(int n) requires(0 <= n && n <= 1000) ensures(r, 0 <= r && r <= 1000 * n) decreases(n) { return n == 0 ? 0 : tri(n - 1) + n; }
    // the bound `1000 * n` is what the induction needs; `variant` 1: the bound `500 * n` (wrong)
    auto mk_tri = [=](Kit& k, int variant, const Expr** body_out)
    {
        Function* f {k.fn("tri", k.i32, {{"n", k.i32}})};
        f->pure = true;
        k.req(f, k.land(k.ge_c(k.F(f, 0), k.i32, "0"), k.le_c(k.F(f, 0), k.i32, "1000")));
        k.ens(f, k.land(k.ge_c(k.R(f), k.i32, "0"), k.le(k.R(f), k.mul(k.num(k.i32, variant == 1 ? "500" : "1000"), k.F(f, 0)))));
        k.dec(f, k.F(f, 0));
        const Expr* self {app(k, f, {k.sub(k.F(f, 0), k.num(k.i32, "1"))}, "tri(n - 1)")};
        *body_out = k.cond(k.eq(k.F(f, 0), k.num(k.i32, "0")), k.num(k.i32, "0"), k.add(self, k.F(f, 0)));
        f->pure_body = *body_out;
        k.done(f);
        return f;
    };
    lc("pure_tri", "tri(n) = 0 + ... + n with its bound: the `ensures` is proved from the function's own induction hypothesis", [=](Kit& k)
       {
           const Expr* b {nullptr};
           mk_tri(k, 0, &b);
       });
    lc("pure_tri_wrong_bound", "the wrong bound 500 * n: `postcondition` at the function's return, one error", [=](Kit& k)
       {
           const Expr* b {nullptr};
           mk_tri(k, 1, &b);
           k.fail("postcondition", b->loc);
       });
    lc("pure_sum_over_read", "the body also reads a[n], one past reads_range(a, n): `permission` in the function, at the read", [=](Kit& k)
       {
           Function* f {mk_sum(k, 3)};
           // the read is the Deref inside the conversion inside the addition (the else arm of the conditional)
           const Expr* read {f->pure_body->c->b->a};
           k.fail("permission", read->loc);
       });
    lc("pure_sum_wrong_bound", "the bound 2147483646 * n is not what the sum satisfies: `postcondition` at the return", [=](Kit& k)
       {
           Function* f {mk_sum(k, 4)};
           k.fail("postcondition", f->pure_body->loc);
       });
    // ---- an application over memory in a `check` (item 7): rows first, then the fact ----
    // int a[3]; a[0..written) = 1, 2, 3; check(sum_to(a, <count>) <op> <bound>);  `fact` 0: <= 6442450941 (what the function's bound gives), 1: < 0 (not provable)
    auto local_check = [=](Kit& k, int written, const char* count, int fact, const Expr** at_app, const Stmt** at_check)
    {
        Function* g {mk_sum(k, 0)};
        Function* f {k.fn("use", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int i {0}; i < written; ++i)
        {
            st.push_back(awr(k, a, N(k, std::to_string(i).c_str()), N(k, std::to_string(i + 1).c_str())));
        }
        const Expr* ap {app(k, g, {k.addr(a), N(k, count)}, "sum_to(a, n)")};
        *at_app = ap;
        const Expr* cond {fact == 0 ? k.le(ap, k.num(k.i64, "6442450941")) : k.lt(ap, k.num(k.i64, "0"))};
        *at_check = k.chk(cond);
        st.push_back(*at_check);
        tail(k, st);
        f->body = k.blkc(st);
        k.done(f);
    };
    lcv("pure_check_local_array", "check(sum_to(a, 3) <= ...) over a local array written in full: the rows, then the fact, verify", [=](Kit& k)
        {
            const Expr* ap {nullptr};
            const Stmt* c {nullptr};
            local_check(k, 3, "3", 0, &ap, &c);
        });
    lc("pure_check_local_false_fact", "check(sum_to(a, 3) < 0): the rows hold, the fact is not provable: `check`", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           const Stmt* c {nullptr};
           local_check(k, 3, "3", 1, &ap, &c);
           k.fail("check", c->loc);
       });
    lc("pure_check_local_unwritten", "the third element is unwritten: the flag row, `uninitialized`, at the application", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           const Stmt* c {nullptr};
           local_check(k, 2, "3", 0, &ap, &c);
           k.fail("uninitialized", ap->loc);
       });
    lc("pure_check_local_count_past", "a count of 4 over an array of 3: the callee's pure line (the bound), `precondition`, one error", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           const Stmt* c {nullptr};
           local_check(k, 3, "4", 0, &ap, &c);
           // the five `precondition` rows are the callee's four atom lines and its value clause; the bound is the fourth, and the row after it is assumed-away
           k.fail_only("precondition", ap->loc, 5, 3);
       });
    lc("pure_check_local_negative_count", "a negative count: the callee's pure line `0 <= n`, `precondition`", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           const Stmt* c {nullptr};
           local_check(k, 3, "-1", 0, &ap, &c);
           // the third row (`0 <= n` of the atom) is the first to fail; what follows is assumed-away (a constant count)
           k.fail_only("precondition", ap->loc, 5, 2);
       });
    // long use(const int *p, int n) requires(0 <= n && reads_range(p, n)) ensures(r, reads_range(p, n)) { check(sum_to(p, n + <extra>) <= ...); return 0; }
    auto param_check = [=](Kit& k, int extra, const Expr** at_app)
    {
        Function* g {mk_sum(k, 0)};
        Function* f {k.fn("use", k.i64, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(k.ge_c(fx(k, f, 1), k.i32, "0"), k.lt(fx(k, f, 1), N(k, "1000"))), RR(k, f, 0, 1, 0)));
        k.ens(f, RR(k, f, 0, 1, 0));
        const Expr* count {extra == 0 ? k.pn(f->param_var(1)) : k.add(k.pn(f->param_var(1)), k.num(k.i32, "1"))};
        const Expr* ap {app(k, g, {k.pn(f->param_var(0)), count}, "sum_to(p, n)")};
        *at_app = ap;
        std::vector<const Stmt*> st {k.chk(k.le(ap, k.num(k.i64, "9223372036854775807"))), k.ret(k.num(k.i64, "0"))};
        f->body = k.blk(st);
        k.done(f);
    };
    lc("pure_check_parameter", "an application in a check over a parameter's reads_range(p, n): the rows (marked wildcard rows over the binder) and the fact verify", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           param_check(k, 0, &ap);
       });
    lc("pure_check_parameter_past", "sum_to(p, n + 1) over reads_range(p, n): the callee's bound line, `precondition`, and the element past the range, `permission` (the marked row is not assumed away)", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           param_check(k, 1, &ap);
           k.fail_only("precondition", ap->loc, 5, 3);
           k.fail_only("permission", ap->loc, 2, 0);
       });
    // ---- an application in the body of another function: the guard, the rows in the caller's `__wf`, the facts ----
    // int clamp100(int x) [ensures(r, 0 <= r && r <= 100)] { return x < 0 ? 0 : (x > 100 ? 100 : x); }
    auto mk_clamp = [=](Kit& k, bool with_ensures)
    {
        Function* f {k.fn("clamp100", k.i32, {{"x", k.i32}})};
        f->pure = true;
        if (with_ensures)
        {
            k.ens(f, k.land(k.ge_c(k.R(f), k.i32, "0"), k.le_c(k.R(f), k.i32, "100")));
        }
        f->pure_body = k.cond(k.lt(k.F(f, 0), k.num(k.i32, "0")), k.num(k.i32, "0"),
                              k.cond(k.gt(k.F(f, 0), k.num(k.i32, "100")), k.num(k.i32, "100"), k.F(f, 0)));
        k.done(f);
        return f;
    };
    // int bump(int x) { return clamp100(x) + 1; }
    auto mk_bump = [=](Kit& k, bool with_ensures, const Expr** sum)
    {
        Function* c {mk_clamp(k, with_ensures)};
        Function* f {k.fn("bump", k.i32, {{"x", k.i32}})};
        f->pure = true;
        *sum = k.add(app(k, c, {k.F(f, 0)}, "clamp100(x)"), k.num(k.i32, "1"));
        f->pure_body = *sum;
        k.done(f);
        return f;
    };
    lc("pure_body_applies_with_ensures", "clamp100(x) + 1: fact (ii) bounds the application, so the sum cannot overflow: the `__wf` rows hold", [=](Kit& k)
       {
           const Expr* sum {nullptr};
           mk_bump(k, true, &sum);
       });
    lc("pure_body_applies_without_ensures", "the same without clamp100's `ensures`: only the type range (fact (i)) is known, so the sum may overflow: `overflow` at the sum, one error", [=](Kit& k)
       {
           const Expr* sum {nullptr};
           mk_bump(k, false, &sum);
           k.fail("overflow", sum->loc);
       });
    // bool wrap(int x) [requires(0 < x)] { return pos(x); }
    auto mk_wrap = [=](Kit& k, bool with_requires, const Expr** ap)
    {
        Function* g {mk_pos(k)};
        Function* f {k.fn("wrap", k.bl, {{"x", k.i32}})};
        f->pure = true;
        if (with_requires)
        {
            k.req(f, k.lt(k.num(k.i32, "0"), k.F(f, 0)));
        }
        *ap = app(k, g, {k.F(f, 0)}, "pos(x)");
        f->pure_body = *ap;
        k.done(f);
        return f;
    };
    lc("pure_body_applies_requires_given", "wrap(x) requires 0 < x applies pos(x): the callee's value clause is a row of the caller's `__wf`, held", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           mk_wrap(k, true, &ap);
       });
    lc("pure_body_applies_requires_not_given", "wrap(x) with no `requires` applies pos(x): the `precondition` row in the function, at the application", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           mk_wrap(k, false, &ap);
           k.fail("precondition", ap->loc);
       });
    // long total(const int *a, int n) requires(0 <= n && reads_range(a, <n or 1>)) { return sum_to(a, n); }
    auto mk_total = [=](Kit& k, bool whole, const Expr** ap)
    {
        Function* g {mk_sum(k, 0)};
        Function* f {k.fn("total", k.i64, {{"a", PI(k)}, {"n", k.i32}})};
        f->pure = true;
        const Expr* atom {whole ? RR(k, f, 0, 1, 0) : k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), N(k, "1"), 0, k.at())};
        k.req(f, k.land(k.ge_c(fx(k, f, 1), k.i32, "0"), atom));
        *ap = app(k, g, {k.F(f, 0), k.F(f, 1)}, "sum_to(a, n)");
        f->pure_body = *ap;
        k.done(f);
        return f;
    };
    lc("pure_body_applies_reads_framed", "total(a, n) over reads_range(a, n) applies sum_to(a, n): the callee's interval is inside the site's, the framing row holds", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           mk_total(k, true, &ap);
       });
    lc("pure_body_applies_reads_unframed", "total over reads_range(a, 1) applies sum_to(a, n): the framing row, `permission`, in the function at the application, one error", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           mk_total(k, false, &ap);
           k.fail("permission", ap->loc);
       });
    // ---- applications in a function's own `requires` and `ensures`, and a cell atom ----
    // bool sp(int x) requires(0 < x && pos(x)) { return x > 1; }
    auto mk_sp = [=](Kit& k)
    {
        Function* g {mk_pos(k)};
        Function* f {k.fn("sp", k.bl, {{"x", k.i32}})};
        f->pure = true;
        k.req(f, k.land(k.lt(k.num(k.i32, "0"), k.F(f, 0)), app(k, g, {k.F(f, 0)}, "pos(x)")));
        f->pure_body = k.gt(k.F(f, 0), k.num(k.i32, "1"));
        k.done(f);
        return f;
    };
    auto sp_check = [=](Kit& k, const char* v, const Expr** ap, const Stmt** c)
    {
        Function* g {mk_sp(k)};
        Function* f {k.fn("use", k.vd)};
        *ap = app(k, g, {k.num(k.i32, v)}, "sp(x)");
        *c = k.chk(*ap);
        std::vector<const Stmt*> st {*c};
        tail(k, st);
        f->body = k.blk(st);
        k.done(f);
    };
    lcv("pure_requires_applies_check", "sp(5) with requires(0 < x && pos(x)): the callee's clause that applies pos is a row of the check, held", [=](Kit& k)
        {
            const Expr* ap {nullptr};
            const Stmt* c {nullptr};
            sp_check(k, "5", &ap, &c);
        });
    lc("pure_requires_applies_check_outside", "sp(500): pos(500) is false, so the second clause of sp's `requires` is not held: `precondition` at the application", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           const Stmt* c {nullptr};
           sp_check(k, "500", &ap, &c);
           k.fail_only("precondition", ap->loc, 2, 1);
       });
    // bool agree(int x) requires(0 < x) ensures(r, r == pos(x)) { return x < 100; }
    auto mk_agree = [=](Kit& k, bool right, const Expr** body)
    {
        Function* g {mk_pos(k)};
        Function* f {k.fn("agree", k.bl, {{"x", k.i32}})};
        f->pure = true;
        k.req(f, k.lt(k.num(k.i32, "0"), k.F(f, 0)));
        k.ens(f, k.eq(k.R(f), app(k, g, {k.F(f, 0)}, "pos(x)")));
        *body = k.lt(k.F(f, 0), k.num(k.i32, right ? "100" : "99"));
        f->pure_body = *body;
        k.done(f);
        return f;
    };
    lc("pure_ensures_applies", "ensures(r, r == pos(x)) with the body pos has: Silicon proves it from the definition", [=](Kit& k)
       {
           const Expr* b {nullptr};
           mk_agree(k, true, &b);
       });
    lc("pure_ensures_applies_wrong", "the body x < 99 differs from pos(x): `postcondition` at the function's return", [=](Kit& k)
       {
           const Expr* b {nullptr};
           mk_agree(k, false, &b);
           k.fail("postcondition", b->loc);
       });
    // int get(const int *p) requires(reads(p)) { return *p; } applied in a check through a pointer variable
    auto mk_get = [=](Kit& k)
    {
        Function* f {k.fn("get", k.i32, {{"p", PI(k)}})};
        f->pure = true;
        k.req(f, k.b.atom(AtomKind::Reads, fx(k, f, 0), 0, k.at()));
        f->pure_body = k.deref(k.F(f, 0));
        k.done(f);
        return f;
    };
    auto get_check = [=](Kit& k, bool write, const Expr** ap, const Stmt** c)
    {
        Function* g {mk_get(k)};
        Function* f {k.fn("use", k.vd)};
        Var* x {k.cell(f, "x", k.i32)};
        Var* p {k.local(f, "p", PI(k))};
        std::vector<const Stmt*> st {k.decl(x, nullptr)};
        if (write)
        {
            st.push_back(k.store(k.addr(x), N(k, "5")));
        }
        st.push_back(k.decl(p, k.addr(x)));
        *ap = app(k, g, {k.pn(p)}, "get(p)");
        *c = k.chk(k.eq(*ap, N(k, "5")));
        st.push_back(*c);
        tail(k, st);
        f->body = k.blkc(st);
        k.done(f);
    };
    lcv("pure_check_cell", "int x = 5; int *p = &x; check(get(p) == 5): the rows of a cell atom (null, liveness, permission, flag) then the fact", [=](Kit& k)
        {
            const Expr* ap {nullptr};
            const Stmt* c {nullptr};
            get_check(k, true, &ap, &c);
        });
    lc("pure_check_cell_unwritten", "x is not written: the flag row, `uninitialized`, at the application", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           const Stmt* c {nullptr};
           get_check(k, false, &ap, &c);
           k.fail("uninitialized", ap->loc);
       });
    // a static assertion in the body of a pure function is a local constant of it, checked by its `__wf` method's prologue
    lc("pure_static_assertion", "bool yes() { static_assert(1 < 2); return true; }: the assertion is a local constant, checked in `__wf`", [=](Kit& k)
       {
           Function* f {k.fn("yes", k.bl)};
           f->pure = true;
           k.lsa(f, k.lt(k.num(k.i32, "1"), k.num(k.i32, "2")));
           f->pure_body = k.yes();
           k.done(f);
       });
    // ---- arguments that read memory (an argument is evaluated once, at the site, with its own rows) ----
    // bool first_small(const int *a, int n) requires(1 <= n && reads_range(a, n)) { return is_small(a[<0 or n>]); }
    auto mk_first_small = [=](Kit& k, bool past, const Expr** read)
    {
        Function* g {mk_small(k)};
        Function* f {k.fn("first_small", k.bl, {{"a", PI(k)}, {"n", k.i32}})};
        f->pure = true;
        k.req(f, k.land(k.ge_c(fx(k, f, 1), k.i32, "1"), RR(k, f, 0, 1, 0)));
        *read = k.deref(k.b.elem_addr(k.F(f, 0), past ? k.F(f, 1) : k.num(k.i32, "0"), k.at()));
        f->pure_body = app(k, g, {*read}, "is_small(a[0])");
        k.done(f);
        return f;
    };
    lc("pure_body_argument_reads", "is_small(a[0]) under reads_range(a, n): the argument's read is framed (its own row), the application's rows hold", [=](Kit& k)
       {
           const Expr* r {nullptr};
           mk_first_small(k, false, &r);
       });
    lc("pure_body_argument_reads_past", "is_small(a[n]): the argument's own framing row, `permission`, at the read, one error", [=](Kit& k)
       {
           const Expr* r {nullptr};
           mk_first_small(k, true, &r);
           k.fail("permission", r->loc);
       });
    lcv("pure_check_argument_reads", "int a[2] = {5}: check(is_small(a[0])): the argument is read as code reads it, then the application", [=](Kit& k)
        {
            Function* g {mk_small(k)};
            Function* f {k.fn("use", k.vd)};
            Var* a {arr(k, f, "a", k.i32, 2U)};
            std::vector<const Stmt*> st {k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "5"))};
            st.push_back(k.chk(app(k, g, {k.deref(aat(k, a, N(k, "0")))}, "is_small(a[0])")));
            tail(k, st);
            f->body = k.blkc(st);
            k.done(f);
        });
    lc("pure_check_argument_reads_unwritten", "the element read as the argument is unwritten: its own `uninitialized` row", [=](Kit& k)
       {
           Function* g {mk_small(k)};
           Function* f {k.fn("use", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 2U)};
           const Expr* rd {k.deref(aat(k, a, N(k, "0")))};
           f->body = k.blkc({k.decl(a, nullptr), k.chk(app(k, g, {rd}, "is_small(a[0])"))});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    // ---- no parameters, other result types ----
    lc("pure_no_parameters", "int seven() { return 7; } with check(seven() == 7): no `requires` line, no guard", [=](Kit& k)
       {
           Function* g {pure_fn(k, "seven", k.i32, {},
                                [&](Function*)
                                {
                                    return k.num(k.i32, "7");
                                })};
           Function* f {k.fn("use", k.vd)};
           f->body = k.blk({k.chk(k.eq(app(k, g, {}, "seven()"), k.num(k.i32, "7")))});
           k.done(f);
       });
    lc("pure_unsigned_char_result", "unsigned char id(unsigned char x) { return x; }: the result tag is `0 255`, fact (i) is the type's range", [=](Kit& k)
       {
           Function* g {pure_fn(k, "id8", k.u8, {{"x", k.u8}},
                                [&](Function* f)
                                {
                                    return k.F(f, 0);
                                })};
           Function* f {k.fn("use", k.vd)};
           f->body = k.blk({k.chk(k.le(app(k, g, {k.num(k.u8, "3")}, "id8(3)"), k.num(k.u8, "255")))});
           k.done(f);
       });
    lc("pure_enumeration_result", "an enumeration result: no result tag and no fact (i) (record 0062 item 8's narrowing); the conversion into the enumeration is a row of the `__wf`", [=](Kit& k)
       {
           const Type e {k.en("E", 32, true, "0", "2")};
           pure_fn(k, "pick", e, {},
                   [&](Function*)
                   {
                       return k.cv(e, k.num(k.i32, "1"));
                   });
       });
    rh("pure_recursion_without_decreases", "applies itself and has no measure", [=](Kit& k)
       {
           mk_sum(k, 1);
       });
    // ---- a quantified clause in a pure function's `requires` (record 0062 items 3, 7 and 8; cut 2) ----
    // small_at(i, a, n): !(0 <= i && i < n) || (-1000 <= a[i] && a[i] <= 1000), named by `cerastes_forall(small_at, a, n)`
    auto small_q = [=](Kit& k, int id, const Expr* a, const Expr* n)
    {
        const Expr* i {k.b.bound(id, k.i32.int_type, "i", k.at())};
        const Expr* x0 {k.b.forall_arg(id, 0, PI(k), a->c_text.empty() ? "a" : a->c_text, k.at())};
        const Expr* x1 {k.b.forall_arg(id, 1, k.i32, n->c_text.empty() ? "n" : n->c_text, k.at())};
        const Expr* rd {k.deref(k.b.elem_addr(x0, i, k.at()))};
        const Expr* in {k.land(k.ge_c(i, k.i32, "0"), k.lt(i, x1))};
        const Expr* body {k.lor(k.lnot(in), k.land(k.ge_c(rd, k.i32, "-1000"), k.le_c(k.deref(k.b.elem_addr(x0, i, k.at())), k.i32, "1000")))};
        return k.b.forall(id, k.i32.int_type, body, {a, n}, "forall(small_at)", "small_at", k.at());
    };
    // int smallsum(const int *a, int n) requires(0 <= n && n <= 1000 && reads_range(a, n) [&& forall(small_at, a, n)]) ensures(r, -1000 * n <= r && r <= 1000 * n) decreases(n)
    //     { return n == 0 ? 0 : smallsum(a, n - 1) + a[n - 1]; }
    auto mk_smallsum = [=](Kit& k, bool with_q, const Expr** add_out)
    {
        Function* f {k.fn("smallsum", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        f->pure = true;
        const Expr* base {k.land(k.land(k.ge_c(fx(k, f, 1), k.i32, "0"), k.le_c(fx(k, f, 1), k.i32, "1000")), RR(k, f, 0, 1, 0))};
        k.req(f, with_q ? k.land(base, small_q(k, 1, fx(k, f, 0), fx(k, f, 1))) : base);
        if (with_q)
        {
            k.ens(f, k.land(k.le(k.mul(k.num(k.i32, "-1000"), k.F(f, 1)), k.R(f)), k.le(k.R(f), k.mul(k.num(k.i32, "1000"), k.F(f, 1)))));
        }
        k.dec(f, k.F(f, 1));
        const Expr* nm1 {k.sub(k.F(f, 1), k.num(k.i32, "1"))};
        const Expr* self {app(k, f, {k.F(f, 0), nm1}, "smallsum(a, n - 1)")};
        const Expr* elem {k.deref(k.b.elem_addr(k.F(f, 0), k.sub(k.F(f, 1), k.num(k.i32, "1")), k.at()))};
        *add_out = k.add(self, elem);
        f->pure_body = k.cond(k.eq(k.F(f, 1), k.num(k.i32, "0")), k.num(k.i32, "0"), *add_out);
        k.done(f);
        return f;
    };
    lc("pure_forall_function", "smallsum with the quantified bound: the function (its quantifier line), its `__wf` (the self-application's quantifier over the ghosts, fact (ii) under V that holds it) verify", [=](Kit& k)
       {
           const Expr* add {nullptr};
           mk_smallsum(k, true, &add);
       });
    lc("pure_forall_function_noq", "no quantified clause and no `ensures`: the sum of the application and a[n - 1] may overflow an int: `overflow` at the sum, in the function", [=](Kit& k)
       {
           const Expr* add {nullptr};
           mk_smallsum(k, false, &add);
           k.fail("overflow", add->loc);
       });
    // int wrap(const int *a, int n) requires(0 <= n && n <= 1000 && reads_range(a, n) [&& forall(small_at, a, n)]) { return smallsum(a, n); }
    auto mk_wrapq = [=](Kit& k, bool with_q, const Expr** ap)
    {
        Function* g {mk_smallsum(k, true, ap)};
        Function* f {k.fn("wrapq", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        f->pure = true;
        const Expr* base {k.land(k.land(k.ge_c(fx(k, f, 1), k.i32, "0"), k.le_c(fx(k, f, 1), k.i32, "1000")), RR(k, f, 0, 1, 0))};
        k.req(f, with_q ? k.land(base, small_q(k, 2, fx(k, f, 0), fx(k, f, 1))) : base);
        *ap = app(k, g, {k.F(f, 0), k.F(f, 1)}, "smallsum(a, n)");
        f->pure_body = *ap;
        k.done(f);
        return f;
    };
    lc("pure_forall_body_applies_given", "wrapq gives the clause: the Q row of its `__wf` holds, and the quantifier is a conjunct of the guard in the Viper function", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           mk_wrapq(k, true, &ap);
       });
    lc("pure_forall_body_applies_not_given", "wrapq does not give the clause: one `precondition` row (the Q row) in its `__wf`, no native error", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           mk_wrapq(k, false, &ap);
           k.fail_only("precondition", ap->loc, 7, 6);
       });
    // int a[3] = {1, 2, <third>}; check(smallsum(a, 3) <= 3000);
    auto small_check = [=](Kit& k, const char* third, const Expr** ap)
    {
        const Expr* add {nullptr};
        Function* g {mk_smallsum(k, true, &add)};
        Function* f {k.fn("use", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")), awr(k, a, N(k, "2"), N(k, third))};
        *ap = app(k, g, {k.addr(a), N(k, "3")}, "smallsum(a, 3)");
        st.push_back(k.chk(k.le(*ap, k.num(k.i32, "3000"))));
        tail(k, st);
        f->body = k.blkc(st);
        k.done(f);
    };
    lcv("pure_forall_check_local", "a local array {1, 2, 3}: the callee's quantified clause is one `precondition` row of the check (with the rows of item 10 for its reads), held; the fact follows from the `ensures`", [=](Kit& k)
        {
            const Expr* ap {nullptr};
            small_check(k, "3", &ap);
        });
    lc("pure_forall_check_local_big", "the third element is 5000: the quantified clause does not hold: one `precondition` row, at the application", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           small_check(k, "5000", &ap);
           k.fail_only("precondition", ap->loc, 7, 6);
       });
    // long use(const int *a, int n) requires(0 <= n && n <= 1000 && reads_range(a, n) [&& forall(small_at, a, n)]) ensures(r, reads_range(a, n)) { check(smallsum(a, n) <= 1000000); return 0; }
    auto param_small = [=](Kit& k, bool with_q, const Expr** ap)
    {
        const Expr* add {nullptr};
        Function* g {mk_smallsum(k, true, &add)};
        Function* f {k.fn("use", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        const Expr* base {k.land(k.land(k.ge_c(fx(k, f, 1), k.i32, "0"), k.le_c(fx(k, f, 1), k.i32, "1000")), RR(k, f, 0, 1, 0))};
        k.req(f, with_q ? k.land(base, small_q(k, 3, fx(k, f, 0), fx(k, f, 1))) : base);
        k.ens(f, RR(k, f, 0, 1, 0));
        *ap = app(k, g, {k.pn(f->param_var(0)), k.pn(f->param_var(1))}, "smallsum(a, n)");
        f->body = k.blk({k.chk(k.le(*ap, k.num(k.i32, "1000000"))), k.ret(k.num(k.i32, "0"))});
        k.done(f);
    };
    lc("pure_forall_check_parameter", "use gives the clause in its `requires`: the check's Q row and the fact verify", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           param_small(k, true, &ap);
       });
    lc("pure_forall_check_parameter_noq", "use does not give the clause: one `precondition` row (the Q row) at the application, no native error", [=](Kit& k)
       {
           const Expr* ap {nullptr};
           param_small(k, false, &ap);
           k.fail_only("precondition", ap->loc, 7, 6);
       });
    (void)PI;
}

// Record 0062 item 5: the rules `check_program` repeats as backstops of the encoder's own rejections.
void define_pure_function_rejections()
{
    auto small = [](Kit& k)
    {
        Function* f {k.fn("is_small", k.bl, {{"x", k.i32}})};
        f->pure = true;
        f->pure_body = k.lt(k.F(f, 0), k.num(k.i32, "100"));
        k.done(f);
        return f;
    };
    auto ap = [](Kit& k, const Function* g, std::vector<const Expr*> args)
    {
        return k.b.apply(g, std::move(args), "is_small(x)", k.at());
    };
    auto caller = [](Kit& k)
    {
        Function* f {k.fn("use", k.vd)};
        return f;
    };
    rc("pure_apply_in_invariant", "outside a `check`", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {caller(k)};
           Var* i {k.local(f, "i", k.i32)};
           f->body = k.blk({k.decl(i, k.num(k.i32, "0")),
                            k.whl(k.lt(k.v(i), k.num(k.i32, "3")), {k.cl(ap(k, g, {k.v(i)}))}, {k.cl(k.sub(k.num(k.i32, "3"), k.v(i)))},
                                  k.blk({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))});
           k.done(f);
       });
    rc("pure_apply_in_ordinary_requires", "outside a `check`", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {k.fn("h", k.vd, {{"x", k.i32}})};
           k.req(f, ap(k, g, {k.F(f, 0)}));
           f->body = k.blk({});
           k.done(f);
       });
    rc("pure_apply_in_ordinary_ensures", "outside a `check`", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {k.fn("h", k.vd, {{"x", k.i32}})};
           k.ens(f, ap(k, g, {k.F(f, 0)}));
           f->body = k.blk({});
           k.done(f);
       });
    rc("pure_apply_in_c_assert", "outside a `check`", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {caller(k)};
           f->body = k.blk({k.asrt(ap(k, g, {k.num(k.i32, "1")}))});
           k.done(f);
       });
    rc("pure_apply_in_measure", "outside a `check`", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* c {k.fn("clamp", k.i32, {{"x", k.i32}})};
           c->pure = true;
           c->pure_body = k.F(c, 0);
           k.done(c);
           Function* f {k.fn("p", k.bl, {{"x", k.i32}})};
           f->pure = true;
           k.dec(f, k.b.apply(c, {k.F(f, 0)}, "clamp(x)", k.at()));
           f->pure_body = ap(k, g, {k.F(f, 0)});
           k.done(f);
       });
    rc("pure_apply_under_old", "under `old`", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {k.fn("p", k.bl, {{"x", k.i32}})};
           f->pure = true;
           k.ens(f, k.old(ap(k, g, {k.F(f, 0)})));
           f->pure_body = ap(k, g, {k.F(f, 0)});
           k.done(f);
       });
    rc("pure_apply_callee_not_pure", "not a `pure` function", [=](Kit& k)
       {
           Function* g {k.fn("g", k.bl, {{"x", k.i32}})};
           g->body = k.blk({k.ret(k.yes())});
           k.done(g);
           Function* f {caller(k)};
           f->body = k.blk({k.chk(ap(k, g, {k.num(k.i32, "1")}))});
           k.done(f);
       });
    rc("pure_apply_arity", "with 0 arguments", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {caller(k)};
           f->body = k.blk({k.chk(ap(k, g, {}))});
           k.done(f);
       });
    rc("pure_apply_argument_type", "(convert it)", [=](Kit& k)
       {
           Function* g {small(k)};
           Function* f {caller(k)};
           f->body = k.blk({k.chk(ap(k, g, {k.num(k.i64, "1")}))});
           k.done(f);
       });
    rc("pure_body_holds_old", "holds an old value", [=](Kit& k)
       {
           Function* f {k.fn("p", k.i32, {{"x", k.i32}})};
           f->pure = true;
           f->pure_body = k.old(k.F(f, 0));
           k.done(f);
       });
    rc("pure_body_holds_atom", "holds a permission atom", [=](Kit& k)
       {
           Function* f {k.fn("p", k.bl, {{"a", k.ptr(k.i32)}})};
           f->pure = true;
           f->pure_body = k.b.atom(AtomKind::Reads, k.F(f, 0), 0, k.at());
           k.done(f);
       });
    rc("pure_body_holds_write", "holds a write or a call", [=](Kit& k)
       {
           Function* f {k.fn("p", k.i32, {{"x", k.i32}})};
           f->pure = true;
           Var* v {k.local(f, "v", k.i32)};
           f->pure_body = k.asgE(v, k.F(f, 0));
           k.done(f);
       });
    rc("pure_requires_other_atom", "other than `reads(*p)` or `reads_range(p, n)`", [=](Kit& k)
       {
           Function* f {k.fn("p", k.i32, {{"a", k.ptr(k.i32)}})};
           f->pure = true;
           k.req(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           f->pure_body = k.num(k.i32, "0");
           k.done(f);
       });
    rc("pure_ensures_atom", "in the `ensures` of the `pure` function", [=](Kit& k)
       {
           Function* f {k.fn("p", k.i32, {{"a", k.ptr(k.i32)}})};
           f->pure = true;
           k.req(f, k.b.atom(AtomKind::Reads, k.F(f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Reads, k.F(f, 0), 0, k.at()));
           f->pure_body = k.num(k.i32, "0");
           k.done(f);
       });
    rc("pure_cycle_mutual", "in a cycle", [=](Kit& k)
       {
           Function* f {k.fn("f", k.bl, {{"x", k.i32}})};
           Function* g {k.fn("g", k.bl, {{"x", k.i32}})};
           f->pure = true;
           g->pure = true;
           f->pure_body = k.b.apply(g, {k.F(f, 0)}, "g(x)", k.at());
           g->pure_body = k.b.apply(f, {k.F(g, 0)}, "f(x)", k.at());
           k.done(f);
           k.done(g);
       });
    rc("pure_cycle_through_ensures", "applies itself in its `requires` or `ensures`", [=](Kit& k)
       {
           Function* f {k.fn("f", k.bl, {{"x", k.i32}})};
           f->pure = true;
           k.ens(f, k.b.apply(f, {k.F(f, 0)}, "f(x)", k.at()));
           f->pure_body = k.yes();
           k.done(f);
       });
}

} // namespace
