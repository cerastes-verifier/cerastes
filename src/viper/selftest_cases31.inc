// (included by selftest.cpp) Quantifiers, M4.5e1c Part B (record 0061 items 3-8 and 11): `forall(f, args...)` in a function's `requires` and `ensures`.  A quantifier in a
// contract lowers to `forall q0.. :: { loc(block_of(p), q0).. } range(i) && q_j == offset_of(p_j) + i + c_j ==> body` (a ghost application in `f__wf`), one row per conjunct, a
// guard outside; at a call the `precondition` row after the callee's atom rows with the pointer bound to a local; at a return the `postcondition` row; in `f__wf` every obligation
// of the body is one quantified row over its read class's binder, reported at the quantifier's site with a note at the operation in `f`, and the reads under a binder are ghost
// functions of a domain tagged `// cerastes:domain-ok 0061` with the assumed facts (a)-(d) of item 7.  A quantifier in a loop invariant, a `check` or code stays refused until Part C.
// Each rule has a case that verifies (with a vacuity twin that fails at a `check(false)` last, so the assumed facts are not inconsistent) and a twin that fails for one reason.
namespace {

void define_quantifiers()
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
    [[maybe_unused]] auto RO = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    // `p[e]` read in a contract of a formal
    [[maybe_unused]] auto crd = [=](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.deref(k.b.elem_addr(fx(k, f, p), idx, k.at()));
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
    auto ard = [=](Kit& k, const Var* a, const Expr* i)
    {
        return k.deref(aat(k, a, i));
    };
    auto awr = [=](Kit& k, const Var* a, const Expr* i, const Expr* v)
    {
        return k.store(aat(k, a, i), v);
    };
    // A positive case and its vacuity twin: the same program with `check(false)` after everything, which must fail.
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the assumed facts are not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    // The quantifier `forall(<fname>, args...)`: the body is built by `body(i, a)` from the bound variable `i` (type int) and the trailing arguments `a` as ForallArg nodes.
    auto mkq = [=](Kit& k, int id, const char* fname, const std::vector<const Expr*>& args,
                   const std::function<const Expr*(const Expr*, const std::vector<const Expr*>&)>& body)
    {
        const Expr* i {k.b.bound(id, k.i32.int_type, "i", k.at())};
        std::vector<const Expr*> a;
        for (size_t j {0U}; j < args.size(); ++j)
        {
            a.push_back(k.b.forall_arg(id, static_cast<int>(j), args[j]->type,
                                       args[j]->c_text.empty() ? "x" : args[j]->c_text, k.at()));
        }
        const Expr* b {body(i, a)};
        return k.b.forall(id, k.i32.int_type, b, args, std::string("forall(") + fname + ")", fname, k.at());
    };
    // `p[idx]` in a body
    auto brd = [=](Kit& k, const Expr* p, const Expr* idx)
    {
        return k.deref(k.b.elem_addr(p, idx, k.at()));
    };
    auto in_range = [=](Kit& k, const Expr* i, const Expr* n)
    {
        return k.land(k.ge_c(i, k.i32, "0"), k.lt(i, n));
    };
    // forall nonneg(a, n): for every i, !(0 <= i && i < n) || a[i] >= 0
    auto nonneg = [=](Kit& k, int id, Function* f, int a, int n)
    {
        return mkq(k, id, "nonneg", {fx(k, f, a), fx(k, f, n)},
                   [&](const Expr* i, const std::vector<const Expr*>& x)
                   {
                       return k.lor(k.lnot(in_range(k, i, x[1])), k.ge_c(brd(k, x[0], i), k.i32, "0"));
                   });
    };
    // the vacuity tail of a program whose last function has no body to end in: a function of its own
    auto vac_fn = [=](Kit& k)
    {
        if (k.vac_tail)
        {
            Function* g {k.fn("vac", k.vd)};
            const Stmt* v {k.chk(k.no())};
            g->body = k.blk({v});
            k.done(g);
            k.vac_loc = v->loc;
        }
    };

    // ---- the callee uses the quantified fact (item 7, S45 (a)) ----
    // int use_nonneg(int *a, int n) requires(reads_range(a, n) && n > 0 && [forall nonneg(a, n)]) ensures(reads_range(a, n) && result >= 0) { return a[0]; }
    const auto use_nonneg = [=](Kit& k, bool with_fact) -> Function*
    {
        Function* f {k.fn("use_nonneg", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        if (with_fact)
        {
            k.req(f, nonneg(k, 0, f, 0, 1));
        }
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(k.R(f), k.i32, "0")));
        const Stmt* r {k.ret(k.deref(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "0"), k.at())))};
        f->body = k.blk({r});
        k.done(f);
        if (!with_fact)
        {
            k.fail("postcondition", r->loc);
        }
        return f;
    };
    lcv("fq_use_nonneg",
        "the callee's requires says every element is non-negative: its return of a[0] meets result >= 0",
        [=](Kit& k)
        {
            use_nonneg(k, true);
            vac_fn(k);
        });
    lc("fq_use_nonneg_nofact",
       "the same without the quantified requires: postcondition at the return (the positive uses the fact)",
       [=](Kit& k)
       {
           use_nonneg(k, false);
       });

    // ---- a caller on a local array: the `precondition` row after the callee's atom rows, the pointer bound to a local ----
    // void c() { int a[4]; a[0] = 1; a[1] = 2; a[2] = [3|-1]; a[3] = 4; use_nonneg(&a[0], 4); [check(false)] }
    const auto caller_nonneg = [=](Kit& k, bool bad) -> const Expr*
    {
        Function* callee {use_nonneg(k, true)};
        Function* f {k.fn("c", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int i {0}; i < 4; ++i)
        {
            st.push_back(
                awr(k, a, N(k, std::to_string(i).c_str()), N(k, bad && i == 2 ? "-1" : std::to_string(i + 1).c_str())));
        }
        const Expr* call {k.call(callee, {aat(k, a, N(k, "0")), N(k, "4")})};
        st.push_back(k.es(call));
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        f->body = k.blkc(std::move(st));
        k.done(f);
        return call;
    };
    lcv("fq_caller_nonneg",
        "int a[4] = 1, 2, 3, 4; use_nonneg(&a[0], 4): the callee's quantified requires holds, the call's precondition row passes",
        [=](Kit& k)
        {
            caller_nonneg(k, false);
        });
    lc("fq_caller_neg", "the same with a[2] == -1: the precondition row of the quantified clause fails at the call",
       [=](Kit& k)
       {
           k.fail_only("precondition", caller_nonneg(k, true)->loc, 2U, 1U);
       });

    // ---- a straight-line proof of a quantified ensures, and a caller that uses it (S45 (a) top3) ----
    // int top3(int *a, int n) requires(reads_range(a, n) && n == 3) ensures(reads_range(a, n) && forall le_r(a, n, result)) { m = a[0]; if (a[1] > m) m = a[1]; if (a[2] > m) m = a[2]; return m; }
    // le_r(a, n, m): for every i, !(0 <= i && i < n) || a[i] <= m   ([<| for the twin)
    const auto top3 = [=](Kit& k, bool strict, Function** fout = nullptr) -> const Stmt*
    {
        Function* f {k.fn("top3", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.eq(fx(k, f, 1), N(k, "3"))));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), mkq(k, 0, strict ? "lt_r" : "le_r", {fx(k, f, 0), fx(k, f, 1), k.R(f)},
                                               [&](const Expr* i, const std::vector<const Expr*>& x)
                                               {
                                                   const Expr* rd {brd(k, x[0], i)};
                                                   return k.lor(k.lnot(in_range(k, i, x[1])),
                                                                strict ? k.lt(rd, x[2]) : k.le(rd, x[2]));
                                               })));
        auto rd = [&](const char* i)
        {
            return k.deref(k.b.elem_addr(k.pn(f->param_var(0)), N(k, i), k.at()));
        };
        Var* m {k.local(f, "m", k.i32)};
        const Stmt* r {k.ret(k.v(m))};
        f->body = k.blkc({k.decl(m, rd("0")), k.iff(k.gt(rd("1"), k.v(m)), k.blkc({k.asg(m, rd("1"))})),
                          k.iff(k.gt(rd("2"), k.v(m)), k.blkc({k.asg(m, rd("2"))})), r});
        k.done(f);
        if (fout != nullptr)
        {
            *fout = f;
        }
        return r;
    };
    lcv("fq_top3",
        "m = max of three elements: the body proves every a[i] <= result at the return, the quantified ensures holds",
        [=](Kit& k)
        {
            top3(k, false);
            vac_fn(k);
        });
    lc("fq_top3_strict", "the same with < in the quantifier: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", top3(k, true)->loc);
       });
    // int a[3]; r = top3(&a[0], 3); check(a[1] [<=|>] r): the caller learns the callee's quantified ensures
    const auto caller_top3 = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* callee {nullptr};
        top3(k, false, &callee);
        Function* f {k.fn("c", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* r {k.local(f, "r", k.i32)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        const int vals[] {5, 9, 7};
        for (int i {0}; i < 3; ++i)
        {
            st.push_back(awr(k, a, N(k, std::to_string(i).c_str()), N(k, std::to_string(vals[i]).c_str())));
        }
        st.push_back(k.decl(r, k.call(callee, {aat(k, a, N(k, "0")), N(k, "3")})));
        const Stmt* c {k.chk(wrong ? k.gt(ard(k, a, N(k, "1")), k.v(r)) : k.le(ard(k, a, N(k, "1")), k.v(r)))};
        st.push_back(c);
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        f->body = k.blkc(std::move(st));
        k.done(f);
        return c;
    };
    lcv("fq_caller_top3", "r = top3(&a[0], 3) then check(a[1] <= r): the callee's quantified ensures is used",
        [=](Kit& k)
        {
            caller_top3(k, false);
        });
    lc("fq_caller_top3_wrong", "the same with check(a[1] > r): the check fails",
       [=](Kit& k)
       {
           k.fail("check", caller_top3(k, true)->loc);
       });

    // `p[e]` of the parameter's local copy, in a body
    auto bdy = [=](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.deref(k.b.elem_addr(k.pn(f->param_var(p)), idx, k.at()));
    };

    // ---- guards: `n == 0 || forall`, `c ? forall : true` (item 3's positive positions) ----
    // void guard_n(int *a, int n) requires(reads_range(a, n) && [n == 0 || forall nonneg(a, n)]) { if (n != 0) check(a[0] >= 0); }
    const auto guard_n = [=](Kit& k, bool fact, bool cond_form) -> const Stmt*
    {
        Function* f {k.fn("guard_n", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        if (fact)
        {
            k.req(f, cond_form ? k.cond(k.ne(fx(k, f, 1), N(k, "0")), nonneg(k, 0, f, 0, 1), k.yes())
                               : k.lor(k.eq(fx(k, f, 1), N(k, "0")), nonneg(k, 0, f, 0, 1)));
        }
        k.ens(f, RR(k, f, 0, 1, 0));
        const Stmt* c {k.chk(k.ge_c(bdy(k, f, 0, N(k, "0")), k.i32, "0"))};
        f->body = k.blkc({k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blkc({c}))});
        k.done(f);
        return c;
    };
    lcv("fq_guard_or", "requires(n == 0 || forall nonneg): the check a[0] >= 0 under n != 0 holds",
        [=](Kit& k)
        {
            guard_n(k, true, false);
            vac_fn(k);
        });
    lc("fq_guard_or_nofact", "the same without the quantified requires: the check fails",
       [=](Kit& k)
       {
           k.fail("check", guard_n(k, false, false)->loc);
       });
    lcv("fq_guard_cond", "requires(n != 0 ? forall nonneg : true): the same check holds",
        [=](Kit& k)
        {
            guard_n(k, true, true);
            vac_fn(k);
        });

    // ---- `old` under the binder (item 8, S45 (e)) ----
    // void set_one(int *a, int n, int j, int v) requires(owns_range(a, n) && 0 <= j && j < n) ensures(owns_range(a, n) && forall same_but(a, n, j)) { a[j] = v; [a[0] = 0;] }
    // same_but(a, n, j): for every i, !(0 <= i && i < n && i != j) || a[i] == old(a[i])
    const auto set_one = [=](Kit& k, bool second, Function** fout) -> Function*
    {
        Function* f {k.fn("set_one", k.vd, {{"a", PI(k)}, {"n", k.i32}, {"j", k.i32}, {"v", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.land(k.ge_c(fx(k, f, 2), k.i32, "0"), k.lt(fx(k, f, 2), fx(k, f, 1)))));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), mkq(k, 0, "same_but", {fx(k, f, 0), fx(k, f, 1), fx(k, f, 2)},
                                               [&](const Expr* i, const std::vector<const Expr*>& x)
                                               {
                                                   return k.lor(k.lnot(k.land(in_range(k, i, x[1]), k.ne(i, x[2]))),
                                                                k.eq(brd(k, x[0], i), k.old(brd(k, x[0], i))));
                                               })));
        std::vector<const Stmt*> body {k.store(k.b.elem_addr(k.pn(f->param_var(0)), k.pv(f, 2), k.at()), k.pv(f, 3))};
        if (second)
        {
            body.push_back(k.store(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "0"), k.at()), N(k, "0")));
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        *fout = f;
        return f;
    };
    lcv("fq_set_one_old",
        "a[j] = v: every other element keeps its entry value, cerastes_old(a[i]) under the binder (the post read and the old read each framed)",
        [=](Kit& k)
        {
            Function* f {nullptr};
            set_one(k, false, &f);
            vac_fn(k);
        });
    lc("fq_set_one_second_write",
       "the same also writing a[0] = 0: the quantified ensures does not hold (postcondition at the closing brace)",
       [=](Kit& k)
       {
           Function* f {nullptr};
           set_one(k, true, &f);
           k.fail("postcondition", f->end_loc);
       });
    // a caller: int a[4] = 1, 2, 3, 4; set_one(&a[0], 4, 2, 9); check(a[0] == 1); check(a[3] == 4)  (twin: check(a[0] == 9))
    const auto caller_set_one = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* callee {nullptr};
        set_one(k, false, &callee);
        Function* f {k.fn("c", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int i {0}; i < 4; ++i)
        {
            st.push_back(awr(k, a, N(k, std::to_string(i).c_str()), N(k, std::to_string(i + 1).c_str())));
        }
        st.push_back(k.es(k.call(callee, {aat(k, a, N(k, "0")), N(k, "4"), N(k, "2"), N(k, "9")})));
        const Stmt* c {k.chk(k.eq(ard(k, a, N(k, "0")), N(k, wrong ? "9" : "1")))};
        st.push_back(c);
        st.push_back(k.chk(k.eq(ard(k, a, N(k, "3")), N(k, "4"))));
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        f->body = k.blkc(std::move(st));
        k.done(f);
        return c;
    };
    lcv("fq_caller_set_one",
        "set_one(&a[0], 4, 2, 9): a[0] and a[3] keep their values (the callee's quantified ensures with old is used)",
        [=](Kit& k)
        {
            caller_set_one(k, false);
        });
    lc("fq_caller_set_one_wrong", "the same checking a[0] == 9: the check fails",
       [=](Kit& k)
       {
           k.fail("check", caller_set_one(k, true)->loc);
       });

    // ---- a trusted declaration with a quantified contract; its caller ----
    // void fill(int *a, int n)  [declared only] requires(owns_range(a, n)) ensures(owns_range(a, n) && forall nonneg(a, n));  caller: int a[4]; fill(&a[0], 4); check(a[1] [>=|<] 0)
    const auto caller_fill = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* fill {k.fn("fill", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(fill, RO(k, fill, 0, 1, 0));
        k.ens(fill, k.land(RO(k, fill, 0, 1, 0), nonneg(k, 0, fill, 0, 1)));
        k.trusted(fill);
        Function* f {k.fn("c", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int i {0}; i < 4; ++i)
        {
            st.push_back(awr(k, a, N(k, std::to_string(i).c_str()), N(k, "-5")));
        }
        st.push_back(k.es(k.call(fill, {aat(k, a, N(k, "0")), N(k, "4")})));
        const Stmt* c {k.chk(wrong ? k.lt(ard(k, a, N(k, "1")), N(k, "0")) : k.ge_c(ard(k, a, N(k, "1")), k.i32, "0"))};
        st.push_back(c);
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        f->body = k.blkc(std::move(st));
        k.done(f);
        return c;
    };
    lcv("fq_trusted_fill",
        "a bodiless fill(a, n) promises every element non-negative: the caller's check(a[1] >= 0) holds after the call",
        [=](Kit& k)
        {
            caller_fill(k, false);
        });
    lc("fq_trusted_fill_wrong",
       "the same checking a[1] < 0: the check fails (the promise is the quantified ensures alone)",
       [=](Kit& k)
       {
           k.fail("check", caller_fill(k, true)->loc);
       });

    // ---- the obligations of the body, at the quantifier's site (item 7, question 247) ----
    // forall in_next(a, n): for every i, !(0 <= i && i < n [- 1]) || a[i + 1] >= 0 (the permission of a[i + 1]: one past the range at i == n - 1 when the guard is `i < n`)
    // void next(int *a, int n) requires(reads_range(a, n) && forall in_next(a, n)) ensures(reads_range(a, n)) {}
    const auto next_fn = [=](Kit& k, bool past, const Expr** read_out = nullptr) -> const Expr*
    {
        Function* f {k.fn("next", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        const Expr* q {mkq(k, 0, "in_next", {fx(k, f, 0), fx(k, f, 1)},
                           [&](const Expr* i, const std::vector<const Expr*>& x)
                           {
                               const Expr* upper {past ? x[1] : k.sub(x[1], N(k, "1"))};
                               const Expr* rd {brd(k, x[0], k.add(i, N(k, "1")))};
                               if (read_out != nullptr)
                               {
                                   *read_out = rd;
                               }
                               return k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(i, upper))),
                                            k.ge_c(rd, k.i32, "0"));
                           })};
        k.req(f, q);
        k.ens(f, RR(k, f, 0, 1, 0));
        f->body = k.blk({});
        k.done(f);
        return q;
    };
    lcv("fq_next_guarded",
        "forall a[i + 1] >= 0 for i < n - 1: the element a[i + 1] is in the range, the guard's n - 1 and the index's i + 1 cannot overflow",
        [=](Kit& k)
        {
            next_fn(k, false);
            vac_fn(k);
        });
    lc("fq_next_past",
       "the same with i < n: a[i + 1] is outside the range at i == n - 1: the permission row at the quantifier's site, named for `in_next`, with a note at the read in it",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           const Expr* q {next_fn(k, true, &rd)};
           Expect e {"permission", q->loc.line, 1U, "", "in `in_next`: "};
           e.note_line = rd->loc.line;
           k.expects.push_back(e);
       });
    // forall plus_one(a, n): for every i, !(0 <= i && i < n) || a[i] + 1 > 0, with [a clause a[i] < 100 before it]
    const auto plus_one = [=](Kit& k, bool bounded) -> const Expr*
    {
        Function* f {k.fn("plus_one", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        if (bounded)
        {
            k.req(f, mkq(k, 0, "lt100", {fx(k, f, 0), fx(k, f, 1)},
                         [&](const Expr* i, const std::vector<const Expr*>& x)
                         {
                             return k.lor(k.lnot(in_range(k, i, x[1])), k.lt(brd(k, x[0], i), N(k, "100")));
                         }));
        }
        const Expr* q {mkq(k, 1, "pos_next", {fx(k, f, 0), fx(k, f, 1)},
                           [&](const Expr* i, const std::vector<const Expr*>& x)
                           {
                               return k.lor(k.lnot(in_range(k, i, x[1])),
                                            k.gt(k.add(brd(k, x[0], i), N(k, "1")), N(k, "0")));
                           })};
        k.req(f, q);
        k.ens(f, RR(k, f, 0, 1, 0));
        f->body = k.blk({});
        k.done(f);
        return q;
    };
    lcv("fq_overflow_rests_on_clause",
        "a[i] + 1 cannot overflow: an earlier quantified clause says a[i] < 100 (the row rests on it)",
        [=](Kit& k)
        {
            plus_one(k, true);
            vac_fn(k);
        });
    lc("fq_overflow_unbounded",
       "the same without the earlier clause: the overflow row of a[i] + 1 fails at the quantifier's site",
       [=](Kit& k)
       {
           k.fail("overflow", plus_one(k, false)->loc);
       });

    // ---- two read classes: neighbours (T2, item 6) ----
    // forall ordered(a, n): for every i, !(0 <= i && i < n - 1) || a[i] <= a[i + 1]
    // int second_ge(int *a, int n) requires(reads_range(a, n) && n >= 2 && forall ordered(a, n)) ensures(reads_range(a, n) && [result == 1|result == 0]) { return a[1] >= a[0]; }
    const auto second_ge = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("second_ge", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "2")));
        k.req(f, mkq(k, 0, "ordered", {fx(k, f, 0), fx(k, f, 1)},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(i, k.sub(x[1], N(k, "1"))))),
                                      k.le(brd(k, x[0], i), brd(k, x[0], k.add(i, N(k, "1")))));
                     }));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.eq(k.R(f), N(k, wrong ? "0" : "1"))));
        const Stmt* r {k.ret(k.bi(k.i32, k.ge(bdy(k, f, 0, N(k, "1")), bdy(k, f, 0, N(k, "0")))))};
        f->body = k.blk({r});
        k.done(f);
        return r;
    };
    lcv("fq_neighbours",
        "a[i] <= a[i + 1] for every i < n - 1 (two classes (a, 0) and (a, 1) of one binder pair): a[1] >= a[0] holds, the result is 1",
        [=](Kit& k)
        {
            second_ge(k, false);
            vac_fn(k);
        });
    lc("fq_neighbours_wrong", "the same claiming result == 0: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", second_ge(k, true)->loc);
       });

    // a caller of second_ge: int a[4] = {1, 2, [2|1], 5}: the callee's neighbour clause (two read classes, the PRE row) holds / fails at the call
    const auto caller_ordered = [=](Kit& k, bool bad) -> const Expr*
    {
        Function* callee {nullptr};
        {
            Function* f {k.fn("second_ge", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
            k.req(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "2")));
            k.req(f, mkq(k, 0, "ordered", {fx(k, f, 0), fx(k, f, 1)},
                         [&](const Expr* i, const std::vector<const Expr*>& x)
                         {
                             return k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(i, k.sub(x[1], N(k, "1"))))),
                                          k.le(brd(k, x[0], i), brd(k, x[0], k.add(i, N(k, "1")))));
                         }));
            k.ens(f, RR(k, f, 0, 1, 0));
            k.trusted(f);
            callee = f;
        }
        Function* f {k.fn("c", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        const int vals[] {1, 2, bad ? 1 : 2, 5};
        for (int i {0}; i < 4; ++i)
        {
            st.push_back(awr(k, a, N(k, std::to_string(i).c_str()), N(k, std::to_string(vals[i]).c_str())));
        }
        const Expr* call {k.call(callee, {aat(k, a, N(k, "0")), N(k, "4")})};
        st.push_back(k.es(call));
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        f->body = k.blkc(std::move(st));
        k.done(f);
        return call;
    };
    lcv("fq_caller_ordered",
        "int a[4] = 1, 2, 2, 5: the callee's neighbour clause (the antecedent ties the second binder to i + 1) holds at the call",
        [=](Kit& k)
        {
            caller_ordered(k, false);
        });
    lc("fq_caller_unordered",
       "the same with 1, 2, 1, 5: the precondition row of the neighbour clause fails at the call",
       [=](Kit& k)
       {
           k.fail_only("precondition", caller_ordered(k, true)->loc, 2U, 1U);
       });

    // ---- two bases: p[i] == q[i] ----
    // int eq_first(int *p, int *q, int n) requires(reads_range(p, n) && reads_range(q, n) && n > 0 && [forall eq(p, q, n)]) ensures(reads_range(p, n) && reads_range(q, n) && result == q[0]) { return p[0]; }
    const auto eq_first = [=](Kit& k, bool fact) -> const Stmt*
    {
        Function* f {k.fn("eq_first", k.i32, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)), k.gt(fx(k, f, 2), N(k, "0"))));
        if (fact)
        {
            k.req(f, mkq(k, 0, "eq", {fx(k, f, 0), fx(k, f, 1), fx(k, f, 2)},
                         [&](const Expr* i, const std::vector<const Expr*>& x)
                         {
                             return k.lor(k.lnot(in_range(k, i, x[2])), k.eq(brd(k, x[0], i), brd(k, x[1], i)));
                         }));
        }
        k.ens(f, k.land(k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)), k.eq(k.R(f), crd(k, f, 1, N(k, "0")))));
        const Stmt* r {k.ret(bdy(k, f, 0, N(k, "0")))};
        f->body = k.blk({r});
        k.done(f);
        return r;
    };
    lcv("fq_two_bases",
        "p[i] == q[i] for every i < n over two reads_range parameters: returning p[0] meets result == q[0]",
        [=](Kit& k)
        {
            eq_first(k, true);
            vac_fn(k);
        });
    lc("fq_two_bases_nofact", "the same without the quantified requires: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", eq_first(k, false)->loc);
       });

    // ---- the reads place keeps its value: fact (d) of item 7 ----
    // int kern(int *a, int n) [declared only] requires([reads|owns]_range(a, n) && forall lt100(a, n)) ensures(same atom && forall succ_le(a, n, result)); succ_le: a[i] + 1 <= m
    // `reads_range` keeps the value (the overflow of the post-state a[i] + 1 rests on the entry's a[i] < 100); `owns_range` does not
    const auto kern = [=](Kit& k, bool owns, const Expr** tail_out = nullptr) -> const Expr*
    {
        Function* f {k.fn("kern", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, owns ? RO(k, f, 0, 1, 0) : RR(k, f, 0, 1, 0));
        k.req(f, mkq(k, 0, "lt100", {fx(k, f, 0), fx(k, f, 1)},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(in_range(k, i, x[1])), k.lt(brd(k, x[0], i), N(k, "100")));
                     }));
        const Expr* q {mkq(k, 1, "succ_le", {fx(k, f, 0), fx(k, f, 1), k.R(f)},
                           [&](const Expr* i, const std::vector<const Expr*>& x)
                           {
                               return k.lor(k.lnot(in_range(k, i, x[1])),
                                            k.le(k.add(brd(k, x[0], i), N(k, "1")), x[2]));
                           })};
        if (tail_out != nullptr)
        {
            // the keep fact's vacuity twin: a last ensures clause whose overflow row fails for the values the clauses allow
            *tail_out = mkq(k, 2, "succ_far", {fx(k, f, 0), fx(k, f, 1)},
                            [&](const Expr* i, const std::vector<const Expr*>& x)
                            {
                                return k.lor(k.lnot(in_range(k, i, x[1])),
                                             k.gt(k.add(brd(k, x[0], i), N(k, "2147483647")), N(k, "0")));
                            });
            k.ens(f, k.land(owns ? RO(k, f, 0, 1, 0) : RR(k, f, 0, 1, 0), k.land(q, *tail_out)));
        }
        else
        {
            k.ens(f, k.land(owns ? RO(k, f, 0, 1, 0) : RR(k, f, 0, 1, 0), q));
        }
        k.trusted(f);
        return q;
    };
    lcv("fq_keep_fact",
        "a reads_range place is returned: its post-state value is its entry value, so a[i] + 1 in the ensures cannot overflow (the entry clause says a[i] < 100)",
        [=](Kit& k)
        {
            kern(k, false);
            vac_fn(k);
        });
    lc("fq_keep_fact_owns",
       "the same over owns_range (no keep fact): the overflow row of the post-state a[i] + 1 fails at the quantifier's site",
       [=](Kit& k)
       {
           k.fail("overflow", kern(k, true)->loc);
       });

    // ---- a later unquantified clause rests on an earlier quantified one: the scalar tie (b) joins the ghost of a[0] to the function's application ----
    // void later(int *a, int n) requires(reads_range(a, n) && [forall lt100(a, n) &&] (n <= 0 || a[0] + 1 > 0)) ensures(reads_range(a, n)) {}
    const auto later = [=](Kit& k, bool fact, const Expr** tail_out = nullptr) -> const Expr*
    {
        Function* f {k.fn("later", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        if (fact)
        {
            k.req(f, mkq(k, 0, "lt100", {fx(k, f, 0), fx(k, f, 1)},
                         [&](const Expr* i, const std::vector<const Expr*>& x)
                         {
                             return k.lor(k.lnot(in_range(k, i, x[1])), k.lt(brd(k, x[0], i), N(k, "100")));
                         }));
        }
        const Expr* sum {k.add(crd(k, f, 0, N(k, "0")), N(k, "1"))};
        k.req(f, k.lor(k.le(fx(k, f, 1), N(k, "0")), k.gt(sum, N(k, "0"))));
        if (tail_out != nullptr)
        {
            // the fact's vacuity twin: a last clause whose obligation fails for the values the facts allow (a[0] < 100), so the assumed facts are not inconsistent
            *tail_out = k.add(crd(k, f, 0, N(k, "0")), N(k, "2147483647"));
            k.req(f, k.lor(k.le(fx(k, f, 1), N(k, "0")), k.gt(*tail_out, N(k, "0"))));
        }
        k.ens(f, RR(k, f, 0, 1, 0));
        f->body = k.blk({});
        k.done(f);
        return sum;
    };
    lcv("fq_later_clause",
        "a[0] + 1 in a later clause cannot overflow: the quantified clause before it says a[0] < 100 (the ground read a[0] is the application at its slot)",
        [=](Kit& k)
        {
            later(k, true);
            vac_fn(k);
        });
    lc("fq_later_clause_nofact", "the same without the quantified clause: the overflow row of a[0] + 1 fails",
       [=](Kit& k)
       {
           k.fail("overflow", later(k, false)->loc);
       });
    lc("fq_later_clause_facts_vacuity",
       "the same with a last clause a[0] + 2147483647 > 0: its overflow row fails (a[0] up to 99 is allowed), exactly that row, so the facts (a), (b) assumed in f__wf are consistent",
       [=](Kit& k)
       {
           const Expr* tail {nullptr};
           later(k, true, &tail);
           k.fail("overflow", tail->loc);
       });
    // forall guard_add(a, n): for every i, !(0 <= i && i + 1 < n) || a[i] >= 0: the guard's own i + 1 may overflow at i == INT_MAX
    lc("fq_guard_arithmetic",
       "the guard's own `i + 1` is an obligation of its own, one row at the quantifier's site (the C overflow at i == INT_MAX)",
       [=](Kit& k)
       {
           Function* f {k.fn("g", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
           k.req(f, RR(k, f, 0, 1, 0));
           const Expr* q {mkq(k, 0, "guard_add", {fx(k, f, 0), fx(k, f, 1)},
                              [&](const Expr* i, const std::vector<const Expr*>& x)
                              {
                                  return k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(k.add(i, N(k, "1")), x[1]))),
                                               k.ge_c(brd(k, x[0], i), k.i32, "0"));
                              })};
           k.req(f, q);
           k.ens(f, RR(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
           k.fail("overflow", q->loc);
       });

    // ---- a read the binder does not index: `a[r]` with `r` a trailing argument (item 7: a scalar ghost tied to the application, a quantified framing row with the body's path) ----
    // int best(int *a, int n, int r) requires(reads_range(a, n) && 0 <= r && r < n && [forall le_at(a, n, r)]) ensures(reads_range(a, n) && result == 1) { return a[0] <= a[r]; }
    // le_at(a, n, r): for every i, !(0 <= i && i < n) || a[i] <= a[r]
    const auto best_fn = [=](Kit& k, bool fact) -> const Stmt*
    {
        Function* f {k.fn("best", k.i32, {{"a", PI(k)}, {"n", k.i32}, {"r", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.land(k.ge_c(fx(k, f, 2), k.i32, "0"), k.lt(fx(k, f, 2), fx(k, f, 1)))));
        if (fact)
        {
            k.req(f, mkq(k, 0, "le_at", {fx(k, f, 0), fx(k, f, 1), fx(k, f, 2)},
                         [&](const Expr* i, const std::vector<const Expr*>& x)
                         {
                             return k.lor(k.lnot(in_range(k, i, x[1])), k.le(brd(k, x[0], i), brd(k, x[0], x[2])));
                         }));
        }
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.eq(k.R(f), N(k, "1"))));
        const Stmt* ret {k.ret(k.bi(k.i32, k.le(bdy(k, f, 0, N(k, "0")), bdy(k, f, 0, k.pv(f, 2)))))};
        f->body = k.blk({ret});
        k.done(f);
        return ret;
    };
    lcv("fq_unindexed_read",
        "a[i] <= a[r] for every i < n, r a trailing argument: the read a[r] the binder does not index is framed under the body's path; a[0] <= a[r] holds",
        [=](Kit& k)
        {
            best_fn(k, true);
            vac_fn(k);
        });
    lc("fq_unindexed_read_nofact", "the same without the quantified requires: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", best_fn(k, false)->loc);
       });

    // ---- a read the binder does not index: `*p` of a pointer argument (item 7: a scalar ghost, a quantified framing row with the body's path) ----
    // int cmp(int *a, int n, int *p) requires(reads_range(a, n) && reads(*p) && n > 0 && forall le_cell(a, n, p)) ensures(reads_range(a, n) && reads(*p) && result == [1|0]) { return a[0] <= *p; }
    // le_cell(a, n, p): for every i, !(0 <= i && i < n) || a[i] <= *p
    const auto cmp_fn = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("cmp", k.i32, {{"a", PI(k)}, {"n", k.i32}, {"p", PI(k)}})};
        k.req(f, k.land(k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())),
                        k.gt(fx(k, f, 1), N(k, "0"))));
        k.req(f, mkq(k, 0, "le_cell", {fx(k, f, 0), fx(k, f, 1), fx(k, f, 2)},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(in_range(k, i, x[1])), k.le(brd(k, x[0], i), k.deref(x[2])));
                     }));
        k.ens(f, k.land(k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())),
                        k.eq(k.R(f), N(k, wrong ? "0" : "1"))));
        const Stmt* r {k.ret(k.bi(k.i32, k.le(bdy(k, f, 0, N(k, "0")), k.deref(k.pn(f->param_var(2))))))};
        f->body = k.blk({r});
        k.done(f);
        return r;
    };
    lcv("fq_cell_read",
        "a[i] <= *p for every i < n with a reads(*p) atom: a read of *p the binder does not index, framed by the atom; a[0] <= *p holds, the result is 1",
        [=](Kit& k)
        {
            cmp_fn(k, false);
            vac_fn(k);
        });
    lc("fq_cell_read_wrong", "the same claiming result == 0: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", cmp_fn(k, true)->loc);
       });

    // ---- two parameters on one block (c): only between pointees of one cell field ----
    // void same_field(int *a, int *b, int n) requires(reads_range(a, n) && reads_range(b, n) && [a == b &&] forall lt100(a, n) && forall pos_next(b, n)) ensures(both atoms) {}
    // with a == b the two parameters are one block: their ghost functions agree, so the overflow row of b[i] + 1 rests on a's clause; without it the row fails
    const auto same_field = [=](Kit& k, bool alias, const Expr** tail_out = nullptr) -> const Expr*
    {
        Function* f {k.fn("same_field", k.vd, {{"a", PI(k)}, {"b", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
        if (alias)
        {
            k.req(f, k.eq(fx(k, f, 0), fx(k, f, 1)));
        }
        k.req(f, mkq(k, 0, "lt100", {fx(k, f, 0), fx(k, f, 2)},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(in_range(k, i, x[1])), k.lt(brd(k, x[0], i), N(k, "100")));
                     }));
        const Expr* q {mkq(k, 1, "pos_next", {fx(k, f, 1), fx(k, f, 2)},
                           [&](const Expr* i, const std::vector<const Expr*>& x)
                           {
                               return k.lor(k.lnot(in_range(k, i, x[1])),
                                            k.gt(k.add(brd(k, x[0], i), N(k, "1")), N(k, "0")));
                           })};
        k.req(f, q);
        if (tail_out != nullptr)
        {
            // the facts' vacuity twin: a last quantified clause over b whose overflow row fails for the values the clauses allow
            *tail_out = mkq(k, 2, "pos_far", {fx(k, f, 1), fx(k, f, 2)},
                            [&](const Expr* i, const std::vector<const Expr*>& x)
                            {
                                return k.lor(k.lnot(in_range(k, i, x[1])),
                                             k.gt(k.add(brd(k, x[0], i), N(k, "2147483647")), N(k, "0")));
                            });
            k.req(f, *tail_out);
        }
        k.ens(f, k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
        f->body = k.blk({});
        k.done(f);
        return q;
    };
    lcv("fq_same_field",
        "a == b makes two int parameters one block: their ghost functions agree, so b[i] + 1 cannot overflow (a's clause says a[i] < 100)",
        [=](Kit& k)
        {
            same_field(k, true);
            vac_fn(k);
        });
    lc("fq_same_field_nofact", "the same without a == b: the overflow row of b[i] + 1 fails at the quantifier's site",
       [=](Kit& k)
       {
           k.fail("overflow", same_field(k, false)->loc);
       });
    lc("fq_same_field_facts_vacuity",
       "the same with a last quantified clause b[i] + 2147483647 > 0: its overflow row fails, exactly that row (the fact (c) between a and b is consistent)",
       [=](Kit& k)
       {
           const Expr* tail {nullptr};
           same_field(k, true, &tail);
           k.fail("overflow", tail->loc);
       });
    lcv("fq_two_fields",
        "an int and a long parameter under quantifiers: no fact ties their ghost functions (one location read as two types has two unrelated values)",
        [=](Kit& k)
        {
            Function* f {k.fn("two_fields", k.vd, {{"a", PI(k)}, {"b", k.ptr(k.i64)}, {"n", k.i32}})};
            k.req(f, k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
            k.req(f, nonneg(k, 0, f, 0, 2));
            k.req(f, mkq(k, 1, "nonneg_l", {fx(k, f, 1), fx(k, f, 2)},
                         [&](const Expr* i, const std::vector<const Expr*>& x)
                         {
                             return k.lor(k.lnot(in_range(k, i, x[1])), k.ge_c(brd(k, x[0], i), k.i64, "0"));
                         }));
            k.ens(f, k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
            f->body = k.blk({});
            k.done(f);
            vac_fn(k);
        });

    lc("fq_keep_fact_facts_vacuity",
       "the same with a last ensures clause a[i] + 2147483647 > 0: its overflow row fails, exactly that row (the keep fact (d) and the ranges (a) are consistent)",
       [=](Kit& k)
       {
           const Expr* tail {nullptr};
           kern(k, false, &tail);
           k.fail("overflow", tail->loc);
       });

    // ---- ground (d) and the `*p` branch of the scalar tie (b): a `reads(*p)` atom with a quantified read of p[i] ----
    // void cellq(int *p) requires([reads|owns](*p) && forall nonneg1(p) [&& (*p - 1 >= -1)]) ensures(same atom && *p - 1 >= -1 && forall nonneg1(p)) {}   nonneg1(p): for every i, !(0 <= i && i < 1) || p[i] >= 0
    // the post-state `*p` is the scalar ghost of the cell, tied to hvq_p_post at its slot (b); a reads place keeps its value (d, ground): with reads the ensures' `*p - 1` cannot underflow (the requires says p[0] >= 0), with owns it can
    const auto cellq = [=](Kit& k, bool owns, const Expr** tail_out = nullptr) -> const Expr*
    {
        Function* f {k.fn("cellq", k.vd, {{"p", PI(k)}})};
        const auto atom = [&]()
        {
            return k.b.atom(owns ? AtomKind::Owns : AtomKind::Reads, fx(k, f, 0), 0, k.at());
        };
        const auto nn1 = [&](int id)
        {
            return mkq(k, id, "nonneg1", {fx(k, f, 0)},
                       [&](const Expr* i, const std::vector<const Expr*>& x)
                       {
                           return k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(i, N(k, "1")))),
                                        k.ge_c(brd(k, x[0], i), k.i32, "0"));
                       });
        };
        k.req(f, k.land(atom(), nn1(0)));
        const Expr* dec {k.sub(k.deref(fx(k, f, 0)), N(k, "1"))};
        k.ens(f, k.land(atom(), k.land(k.ge_c(dec, k.i32, "-1"), nn1(1))));
        if (tail_out != nullptr)
        {
            *tail_out = k.add(k.deref(fx(k, f, 0)), N(k, "2147483647"));
            k.ens(f, k.gt(*tail_out, N(k, "0")));
        }
        f->body = k.blk({});
        k.done(f);
        return dec;
    };
    lcv("fq_cell_keep",
        "reads(*p) is returned: its post-state value is its entry value (the ground fact (d), the scalar tie (b) of *p), so *p - 1 >= -1 in the ensures cannot underflow",
        [=](Kit& k)
        {
            cellq(k, false);
            vac_fn(k);
        });
    lc("fq_cell_keep_owns", "the same over owns(*p): no keep fact, the underflow row of *p - 1 fails",
       [=](Kit& k)
       {
           k.fail("overflow", cellq(k, true)->loc);
       });
    lc("fq_cell_keep_facts_vacuity",
       "the same over reads(*p) with a last ensures *p + 2147483647 > 0: its overflow row fails, exactly that row (the ground fact (d) is consistent)",
       [=](Kit& k)
       {
           const Expr* tail {nullptr};
           cellq(k, false, &tail);
           k.fail("overflow", tail->loc);
       });

    // ---- a class read in both states: the trigger is the first read's ghost (the post state here) ----
    // int both(int *a, int n) [declared only] requires(owns_range(a, n)) ensures(owns_range(a, n) && forall both_states(a, n) && (n <= 0 || old(a[0]) + 1 > 0))
    // both_states(a, n): for every i, !(0 <= i && i < n) || (a[i] >= 0 && old(a[i]) < 100); the last clause names only the pre state: its overflow row rests on old(a[0]) < 100
    const auto both_fn = [=](Kit& k, const Expr** sum_out, bool bounded) -> void
    {
        Function* f {k.fn("both", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, RO(k, f, 0, 1, 0));
        const Expr* sum {k.add(k.old(crd(k, f, 0, N(k, "0"))), N(k, "1"))};
        *sum_out = sum;
        k.ens(f, k.land(RO(k, f, 0, 1, 0),
                        k.land(mkq(k, 0, "both_states", {fx(k, f, 0), fx(k, f, 1)},
                                   [&](const Expr* i, const std::vector<const Expr*>& x)
                                   {
                                       return k.lor(k.lnot(in_range(k, i, x[1])),
                                                    k.land(k.ge_c(brd(k, x[0], i), k.i32, "0"),
                                                           bounded ? k.lt(k.old(brd(k, x[0], i)), N(k, "100"))
                                                                   : k.ge_c(k.old(brd(k, x[0], i)), k.i32, "0")));
                                   }),
                               k.lor(k.le(fx(k, f, 1), N(k, "0")), k.gt(sum, N(k, "0"))))));
        k.trusted(f);
    };
    lcv("fq_both_states_pre_only",
        "a clause reads a[i] (post) and old(a[i]) (pre) over owns_range, a later clause names only old(a[0]): the class has one trigger set per state in f__wf, so old(a[0]) + 1 cannot overflow (the clause says old(a[i]) < 100)",
        [=](Kit& k)
        {
            const Expr* sum {nullptr};
            both_fn(k, &sum, true);
            vac_fn(k);
        });
    lc("fq_both_states_pre_only_unbounded",
       "the same with old(a[i]) >= 0 in the clause (no upper bound): the overflow row of old(a[0]) + 1 fails",
       [=](Kit& k)
       {
           const Expr* sum {nullptr};
           both_fn(k, &sum, false);
           k.fail("overflow", sum->loc);
       });

    // ---- the trailing arguments' obligations, once, at the site (audit round 1, B1) ----
    // void argobl(int *a, int n) requires(reads_range(a, n) && forall ignore_m(a, n, [n - 1|n + 1])) ensures(reads_range(a, n)) {}; ignore_m never uses its third parameter
    const auto argobl = [=](Kit& k, bool overflow) -> const Expr*
    {
        Function* f {k.fn("argobl", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        const Expr* arg {overflow ? k.add(fx(k, f, 1), N(k, "1")) : k.sub(fx(k, f, 1), N(k, "1"))};
        k.req(f, mkq(k, 0, "ignore_m", {fx(k, f, 0), fx(k, f, 1), arg},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(in_range(k, i, x[1])), k.ge_c(brd(k, x[0], i), k.i32, "0"));
                     }));
        k.ens(f, RR(k, f, 0, 1, 0));
        f->body = k.blk({});
        k.done(f);
        return arg;
    };
    lcv("fq_arg_obligation",
        "forall ignore_m(a, n, n - 1): the argument cannot underflow (0 <= n from the atom); f never uses it",
        [=](Kit& k)
        {
            argobl(k, false);
            vac_fn(k);
        });
    lc("fq_arg_overflow",
       "the same with n + 1: the overflow of the argument is a row of the site whether or not f uses the parameter",
       [=](Kit& k)
       {
           k.fail("overflow", argobl(k, true)->loc);
       });

    // ---- the elements' types: Bool and Ref ghosts in f__wf (audit round 1, S3) ----
    // int all_true(bool *b, int n) requires(reads_range(b, n) && n > 0 && forall istrue(b, n)) ensures(reads_range(b, n) && result == [1|0]) { return b[0]; }
    const auto all_true = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("all_true", k.i32, {{"b", k.ptr(k.bl)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.req(f, mkq(k, 0, "istrue", {fx(k, f, 0), fx(k, f, 1)},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(in_range(k, i, x[1])), brd(k, x[0], i));
                     }));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.eq(k.R(f), N(k, wrong ? "0" : "1"))));
        const Stmt* r {k.ret(k.bi(k.i32, bdy(k, f, 0, N(k, "0"))))};
        f->body = k.blk({r});
        k.done(f);
        return r;
    };
    lcv("fq_bool_ghost",
        "bool b[i] under the binder (a Bool ghost in f__wf, no type range): returning b[0] meets result == 1",
        [=](Kit& k)
        {
            all_true(k, false);
            vac_fn(k);
        });
    lc("fq_bool_ghost_wrong", "the same claiming result == 0: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", all_true(k, true)->loc);
       });
    // int nn(int **pp, int n) requires(reads_range(pp, n) && n > 0 && forall nonnull(pp, n)) ensures(reads_range(pp, n) && result == [1|0]) { return 1; }
    const auto nn_fn = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("nn", k.i32, {{"pp", k.ptr(PI(k))}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.req(f, mkq(k, 0, "nonnull", {fx(k, f, 0), fx(k, f, 1)},
                     [&](const Expr* i, const std::vector<const Expr*>& x)
                     {
                         return k.lor(k.lnot(in_range(k, i, x[1])), k.ne(brd(k, x[0], i), k.nul(PI(k))));
                     }));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.eq(k.R(f), N(k, wrong ? "0" : "1"))));
        const Stmt* r {k.ret(N(k, "1"))};
        f->body = k.blk({r});
        k.done(f);
        return r;
    };
    lcv("fq_ref_ghost",
        "int *pp[i] != NULL under the binder (a Ref ghost in f__wf, its range the pointer's well-formedness): only the contract reads the pointer elements: result == 1",
        [=](Kit& k)
        {
            nn_fn(k, false);
            vac_fn(k);
        });
    lc("fq_ref_ghost_wrong", "the same claiming result == 0: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", nn_fn(k, true)->loc);
       });

    // ---- a marked site frames no quantified read (audit round 1, S7; 0042 item 6) ----
    // void mk(int *p, int *q, int n, int m) requires(owns_range(p, n) && owns_range(q, m) && m == 0 && q == p && forall nonneg(q, n)) ensures(owns_range(p, n) && owns_range(q, m));
    // with p marked ref_to_uninit its atom frames nothing; q's own atom holds no slot (m == 0), so the read through the alias q is framed by p's atom alone
    const auto marked_site = [=](Kit& k, bool marked) -> const Expr*
    {
        Function* f {k.fn("mk", k.vd, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}, {"m", k.i32}})};
        if (marked)
        {
            f->params[0].init = InitMarker::RefToUninit;
        }
        k.req(f, k.land(k.land(RO(k, f, 0, 2, 0), RO(k, f, 1, 3, 1)),
                        k.land(k.eq(fx(k, f, 3), N(k, "0")), k.eq(fx(k, f, 1), fx(k, f, 0)))));
        const Expr* q {nonneg(k, 0, f, 1, 2)};
        k.req(f, q);
        k.ens(f, k.land(RO(k, f, 0, 2, 0), RO(k, f, 1, 3, 1)));
        f->body = k.blk({});
        k.done(f);
        return q;
    };
    lcv("fq_unmarked_site", "owns_range(p, n) with q == p frames the quantified read of q[i]: the framing row holds",
        [=](Kit& k)
        {
            marked_site(k, false);
            vac_fn(k);
        });
    lc("fq_marked_site",
       "the same with p marked ref_to_uninit: the marked atom frames no read of the entry state, so the read through the alias q fails the framing at the site",
       [=](Kit& k)
       {
           k.fail("permission", marked_site(k, true)->loc);
       });
    // ---- a formal with no atom of its own: an alias q == p read under a binder (it keeps its framing row, loses facts (a), (b), (c)) ----
    // void al(int *p, int *q, int n) requires(reads_range(p, n) && [q == p &&] n > 0 && forall nonneg(q, n)) ensures(reads_range(p, n)) {}
    const auto alias_fn = [=](Kit& k, bool alias) -> const Expr*
    {
        Function* f {k.fn("al", k.vd, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 2, 0), alias ? k.eq(fx(k, f, 1), fx(k, f, 0)) : k.yes()));
        const Expr* q {nonneg(k, 0, f, 1, 2)};
        k.req(f, q);
        k.ens(f, RR(k, f, 0, 2, 0));
        f->body = k.blk({});
        k.done(f);
        return q;
    };
    lcv("fq_alias_no_atom",
        "q == p, p's reads_range frames the quantified read of q[i]; q has no atom, so no range fact for its ghost (nothing here needs it)",
        [=](Kit& k)
        {
            alias_fn(k, true);
            vac_fn(k);
        });
    lc("fq_alias_no_atom_unframed",
       "the same without q == p: nothing frames q[i], the permission row fails at the site",
       [=](Kit& k)
       {
           k.fail("permission", alias_fn(k, false)->loc);
       });

    // a quantified read through the marked parameter itself is refused (0042 item 6), as the direct read is
    rc("fq_rej_marked_read", "is marked `ref_to_uninit` or `must_init` and a contract reads `*p`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, RO(k, f, 0, 1, 0));
           k.req(f, nonneg(k, 0, f, 0, 1));
           k.ens(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });

    // ---- the rejections of check_program (item 5's backstop) ----
    // a function f(int *a, int n) whose requires holds `bad(a, n)`
    auto rq = [=](const char* name, const char* why, const std::function<const Expr*(Kit&, Function*)>& make)
    {
        rc(name, why,
           [=](Kit& k)
           {
               Function* f {k.fn("f", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
               k.req(f, RR(k, f, 0, 1, 0));
               k.req(f, make(k, f));
               f->body = k.blk({});
               k.done(f);
           });
    };
    rq("fq_rej_negated", "not positive",
       [=](Kit& k, Function* f)
       {
           return k.lnot(nonneg(k, 0, f, 0, 1));
       });
    rq("fq_rej_operand_of_eq", "not positive",
       [=](Kit& k, Function* f)
       {
           return k.eq(nonneg(k, 0, f, 0, 1), k.yes());
       });
    rq("fq_rej_left_of_or", "on the left of `||`",
       [=](Kit& k, Function* f)
       {
           return k.lor(nonneg(k, 0, f, 0, 1), nonneg(k, 1, f, 0, 1));
       });
    rq("fq_rej_nested", "inside a quantifier",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "outer", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          const Expr* inner {mkq(k, 1, "inner", {x[0], x[1]},
                                                 [&](const Expr* j, const std::vector<const Expr*>& y)
                                                 {
                                                     return k.ge_c(brd(k, y[0], j), k.i32, "0");
                                                 })};
                          return k.lor(k.lnot(in_range(k, i, x[1])),
                                       k.land(inner, k.ge_c(brd(k, x[0], i), k.i32, "0")));
                      });
       });
    rq("fq_rej_formal_in_body", "does not admit",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "formal", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          return k.lor(k.lnot(in_range(k, i, fx(k, f, 1))), k.ge_c(brd(k, x[0], i), k.i32, "0"));
                      });
       });
    rq("fq_rej_no_class", "no element read indexed by the bound variable",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "noclass", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          return k.lor(k.lnot(in_range(k, i, x[1])), k.ge_c(brd(k, x[0], N(k, "0")), k.i32, "0"));
                      });
       });
    rq("fq_rej_index_form", "another form than `i`, `i + c` or `i - c`",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "twice", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          return k.lor(k.lnot(in_range(k, i, x[1])),
                                       k.ge_c(brd(k, x[0], k.mul(N(k, "2"), i)), k.i32, "0"));
                      });
       });
    rq("fq_rej_base_not_parameter", "not a pointer parameter",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "nullbase", {k.nul(PI(k)), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          return k.lor(k.lnot(in_range(k, i, x[1])), k.ge_c(brd(k, x[0], i), k.i32, "0"));
                      });
       });
    rq("fq_rej_argument_reads_memory", "reads memory",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "memarg", {fx(k, f, 0), crd(k, f, 0, N(k, "0"))},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          return k.lor(k.lnot(in_range(k, i, x[1])), k.ge_c(brd(k, x[0], i), k.i32, "0"));
                      });
       });
    rq("fq_rej_foreign_argument", "outside its quantifier",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "foreign", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          const Expr* stray {k.b.forall_arg(9, 1, k.i32, "n", k.at())};
                          return k.lor(k.lnot(in_range(k, i, stray)), k.ge_c(brd(k, x[0], i), k.i32, "0"));
                      });
       });
    rc("fq_rej_under_old", "not positive",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, k.land(RO(k, f, 0, 1, 0), k.old(nonneg(k, 0, f, 0, 1))));
           f->body = k.blk({});
           k.done(f);
       });
    rq("fq_rej_atom_guard", "a permission atom in a position decision 0037 item 5 does not allow",
       [=](Kit& k, Function* f)
       {
           // forall(...) || owns_range(a, n): a quantifier as the guard of an atom
           return k.lor(nonneg(k, 0, f, 0, 1), RO(k, f, 0, 1, 1));
       });
    // a quantifier with several read classes: no guard that reads memory, no arithmetic over two classes
    rq("fq_rej_multi_guard_reads", "a guard that reads memory in a quantifier with several read classes",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "guard_reads", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          const Expr* g {k.land(k.ge_c(i, k.i32, "0"), k.lt(i, k.sub(x[1], N(k, "1"))))};
                          return k.lor(k.lnot(k.land(g, k.gt(brd(k, x[0], i), N(k, "0")))),
                                       k.gt(brd(k, x[0], k.add(i, N(k, "1"))), N(k, "0")));
                      });
       });
    rq("fq_rej_multi_arithmetic", "arithmetic over the reads of two classes in a quantifier with several read classes",
       [=](Kit& k, Function* f)
       {
           return mkq(k, 0, "diff", {fx(k, f, 0), fx(k, f, 1)},
                      [&](const Expr* i, const std::vector<const Expr*>& x)
                      {
                          const Expr* g {k.land(k.ge_c(i, k.i32, "0"), k.lt(i, k.sub(x[1], N(k, "1"))))};
                          return k.lor(k.lnot(g),
                                       k.ge_c(k.sub(brd(k, x[0], k.add(i, N(k, "1"))), brd(k, x[0], i)), k.i32, "0"));
                      });
       });
    rc("fq_rej_old_in_requires", "`old` outside an `ensures`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.req(f, mkq(k, 0, "oldreq", {fx(k, f, 0), fx(k, f, 1)},
                        [&](const Expr* i, const std::vector<const Expr*>& x)
                        {
                            return k.lor(k.lnot(in_range(k, i, x[1])), k.eq(brd(k, x[0], i), k.old(brd(k, x[0], i))));
                        }));
           f->body = k.blk({});
           k.done(f);
       });
    rc("fq_rej_old_argument", "holds `old`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, k.land(RO(k, f, 0, 1, 0), mkq(k, 0, "oldarg", {fx(k, f, 0), k.old(fx(k, f, 1))},
                                                  [&](const Expr* i, const std::vector<const Expr*>& x)
                                                  {
                                                      return k.lor(k.lnot(in_range(k, i, x[1])),
                                                                   k.ge_c(brd(k, x[0], i), k.i32, "0"));
                                                  })));
           f->body = k.blk({});
           k.done(f);
       });

    // ---- the refusal that stays: a quantifier in code (a loop invariant and a `check` are Part C's, selftest_cases32.inc) ----
    const char* const refused {"a quantifier outside the `requires` and `ensures`"};
    auto fa_in_code = [=](Kit& k, Function* f)
    {
        // a quantifier over a local array's address: refused wherever it stands outside a contract
        Var* a {arr(k, f, "a", k.i32, 4U)};
        return std::make_pair(a, mkq(k, 0, "nonneg", {k.addr(a), N(k, "4")},
                                     [&](const Expr* i, const std::vector<const Expr*>& x)
                                     {
                                         return k.lor(k.lnot(in_range(k, i, x[1])),
                                                      k.ge_c(brd(k, x[0], i), k.i32, "0"));
                                     }));
    };
    rc("fq_rej_in_code", refused,
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           auto [a, q] {fa_in_code(k, f)};
           Var* b {k.local(f, "b", Type::bool_type())};
           f->body = k.blk({k.decl(a, nullptr), k.decl(b, q)});
           k.done(f);
       });
    rc("fq_rej_bound_alone", "a quantifier's bound variable outside its quantifier",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           f->body = k.blk({k.ret(k.b.bound(0, k.i32.int_type, "i", k.at()))});
           k.done(f);
       });
}

} // namespace
