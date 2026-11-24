// (included by selftest.cpp) Quantifiers, M4.5e1c Part C (record 0061 items 9 and 10): `forall(f, args...)` in a loop invariant and in a `check`.  In an invariant the quantified clause comes after the
// loop's own conjuncts and the framing of each read under the binder (a quantified assert at entry and at the back edge, `permission`), then one invariant line per obligation (a named array's bound,
// `out-of-bounds`; the flag, `uninitialized`, under the array's bound), each over its read class's binder; in a `check` the rows of each read class (the null and liveness rows of a pointer base, the bound,
// the marked `wildcard` rows for the value and flag fields, the flag row) come before the fact.  Each rule has a case that verifies (with a vacuity twin) and a twin that fails for one reason.
namespace {

void define_quantifier_loops()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.b.text(k.num(k.i32, v), v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
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
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
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
    // The quantifier `forall(<fname>, args...)`: `ptypes` are `f`'s parameter types after the bound variable (an array argument is given for a `T *` parameter), the body is built by `body(i, a)`.
    auto mkq = [=](Kit& k, int id, const char* fname, const std::vector<const Expr*>& args,
                   const std::vector<Type>& ptypes,
                   const std::function<const Expr*(const Expr*, const std::vector<const Expr*>&)>& body)
    {
        const Expr* i {k.b.bound(id, k.i32.int_type, "i", k.at())};
        std::vector<const Expr*> a;
        for (size_t j {0U}; j < args.size(); ++j)
        {
            a.push_back(k.b.forall_arg(id, static_cast<int>(j), ptypes[j],
                                       args[j]->c_text.empty() ? "x" : args[j]->c_text, k.at()));
        }
        const Expr* b {body(i, a)};
        return k.b.forall(id, k.i32.int_type, b, args, std::string("forall(") + fname + ")", fname, k.at());
    };
    auto brd = [=](Kit& k, const Expr* p, const Expr* idx)
    {
        return k.deref(k.b.elem_addr(p, idx, k.at()));
    };
    auto in_range = [=](Kit& k, const Expr* i, const Expr* n)
    {
        return k.land(k.ge_c(i, k.i32, "0"), k.lt(i, n));
    };
    // the vacuity tail: a function of its own that ends in `check(false)` (a program whose last function cannot end in it)
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
    // forall is_idx(a, n): for every j, !(0 <= j && j < n) || a[j] == j
    auto is_idx = [=](Kit& k, int id, const Expr* a, const Expr* n)
    {
        return mkq(k, id, "is_idx", {a, n}, {PI(k), k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       return k.lor(k.lnot(in_range(k, j, x[1])), k.eq(brd(k, x[0], j), j));
                   });
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto RO = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    [[maybe_unused]] auto RR = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    // the parameter's local copy as a base: `p[e]`, a read and a store
    auto bat = [](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.b.elem_addr(k.pn(f->param_var(p)), idx, k.at());
    };
    [[maybe_unused]] auto prd = [=](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.deref(bat(k, f, p, idx));
    };
    auto pwr = [=](Kit& k, Function* f, int p, const Expr* idx, const Expr* v)
    {
        return k.store(bat(k, f, p, idx), v);
    };
    // ---- the fill loop (S45 (c)): int a[3]; int i = 0; while (i < 3) inv 0 <= i <= 3 && forall is_idx(a, i) { a[i] = i; i++; } check(a[1] == 1); ----
    // `mode`: 0 the fill loop, 1 the body skips the write, 2 it writes i + 1, 3 the invariant reads one element too far (forall is_idx(a, i + 1))
    struct Fill
    {
        const Expr* q {nullptr};
        SrcLoc clause;
    };
    // `form`: 0 two clauses (the range, the quantifier), 1 one clause `range && forall` (the quantifier last), 2 one clause `forall && range` (first), as `CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(...))`
    const auto fill = [=](Kit& k, int mode, Fill* info = nullptr, int form = 0)
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* n {mode == 3 ? k.add(k.v(i), N(k, "1")) : k.v(i)};
        const Expr* q {is_idx(k, 0, k.addr(a), n)};
        const Stmt* w {awr(k, a, k.v(i), mode == 2 ? k.add(k.v(i), N(k, "1")) : k.v(i))};
        std::vector<const Stmt*> body;
        if (mode != 1)
        {
            body.push_back(w);
        }
        body.push_back(inc(k, i));
        const Expr* rng {k.range(k.v(i), k.i32, "0", "3")};
        const Clause user {k.cl(form == 0 ? q : form == 1 ? k.land(rng, q) : k.land(q, rng))};
        std::vector<Clause> invs;
        if (form == 0)
        {
            invs = {k.cl(rng), user};
        }
        else
        {
            invs = {user};
        }
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), invs, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(body))};
        const Stmt* c {k.chk(k.eq_c(ard(k, a, N(k, "1")), k.i32, "1"))};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")), lp, c});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_fill_local",
        "the fill loop over int a[3]: the invariant says every element below i holds its index, its flag lines and bound line hold, and a[1] == 1 follows after the loop",
        [=](Kit& k)
        {
            fill(k, 0);
            vac_fn(k);
        });
    // the clause is a conjunction holding the quantifier, as users write it: one invariant line per top-level conjunct, in order, each reported at the clause
    lcv("fql_fill_conjunction_last",
        "CERASTES_INVARIANT(0 <= i && i <= 3 && forall is_idx(a, i)): the clause is split into the range line and the quantifier's own lines, in order",
        [=](Kit& k)
        {
            fill(k, 0, nullptr, 1);
            vac_fn(k);
        });
    lcv("fql_fill_conjunction_first",
        "CERASTES_INVARIANT(forall is_idx(a, i) && 0 <= i && i <= 3): the quantifier first, its lines before the range's",
        [=](Kit& k)
        {
            fill(k, 0, nullptr, 2);
            vac_fn(k);
        });
    lc("fql_fill_conjunction_value",
       "the conjunction form with the body writing i + 1: the quantifier's line fails (invariant-preserved at the clause, one error: the range line holds)",
       [=](Kit& k)
       {
           Fill fi;
           fill(k, 2, &fi, 1);
           k.fail_only("invariant-preserved", fi.clause, 2U, 1U);
       });
    lc("fql_fill_conjunction_value_first",
       "the same with the quantifier first (invariant-preserved at the clause, one error)",
       [=](Kit& k)
       {
           Fill fi;
           fill(k, 2, &fi, 2);
           k.fail_only("invariant-preserved", fi.clause, 2U, 0U);
       });
    lc("fql_fill_conjunction_skip",
       "the conjunction form with the write skipped: the flag line (uninitialized) and the quantifier's line (invariant-preserved) fail, as for two clauses",
       [=](Kit& k)
       {
           Fill fi;
           fill(k, 1, &fi, 1);
           k.fail_only("uninitialized", fi.q->loc, 2U, 1U);
           k.fail_only("invariant-preserved", fi.clause, 2U, 1U);
       });
    lc("fql_fill_local_skip",
       "the body skips the write: at the back edge the new element's flag and value are both unknown, so the flag line (uninitialized) and the user's clause (invariant-preserved) each fail (question 254: two errors, both real)",
       [=](Kit& k)
       {
           Fill fi;
           fill(k, 1, &fi);
           k.fail_only("uninitialized", fi.q->loc, 2U, 1U);
           k.fail("invariant-preserved", fi.clause);
       });
    lc("fql_fill_local_value",
       "the body writes i + 1: the flag line holds, the value clause fails (invariant-preserved, one error)",
       [=](Kit& k)
       {
           Fill fi;
           fill(k, 2, &fi);
           k.fail("invariant-preserved", fi.clause);
       });
    lc("fql_fill_local_far",
       "the invariant is stated for i + 1 elements: at entry a[0] has no value, so the flag line fails (uninitialized, at entry)",
       [=](Kit& k)
       {
           Fill fi;
           fill(k, 3, &fi);
           k.fail_only("uninitialized", fi.q->loc, 2U, 0U);
       });

    // ---- the bodies of the quantifiers below: `!(0 <= j && j < n) || E` over `j` ----
    // every element below `n` is at most `m` (a value argument): forall le_val(a, n, m) = a[j] <= m
    [[maybe_unused]] auto le_val = [=](Kit& k, int id, const Expr* a, const Expr* n, const Expr* m, bool strict)
    {
        return mkq(k, id, "le_val", {a, n, m}, {PI(k), k.i32, k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       const Expr* r {brd(k, x[0], j)};
                       return k.lor(k.lnot(in_range(k, j, x[1])), strict ? k.lt(r, x[2]) : k.le(r, x[2]));
                   });
    };
    // every element below `n` is at most a[r] (a read the binder does not index): forall le_at(a, n, r) = a[j] <= a[r]
    [[maybe_unused]] auto le_at = [=](Kit& k, int id, const Expr* a, const Expr* n, const Expr* r)
    {
        return mkq(k, id, "le_at", {a, n, r}, {PI(k), k.i32, k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       return k.lor(k.lnot(in_range(k, j, x[1])), k.le(brd(k, x[0], j), brd(k, x[0], x[2])));
                   });
    };
    // forall ne_val(a, n, x) = a[j] != x
    [[maybe_unused]] auto ne_val = [=](Kit& k, int id, const Expr* a, const Expr* n, const Expr* v, bool eq)
    {
        return mkq(k, id, "ne_val", {a, n, v}, {PI(k), k.i32, k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       const Expr* r {brd(k, x[0], j)};
                       return k.lor(k.lnot(in_range(k, j, x[1])), eq ? k.eq(r, x[2]) : k.ne(r, x[2]));
                   });
    };
    // neighbours: forall nondec(a, n) = !(0 <= j && j < n - 1) || a[j] <= a[j + 1]
    [[maybe_unused]] auto nondec = [=](Kit& k, int id, const Expr* a, const Expr* n)
    {
        return mkq(k, id, "nondec", {a, n}, {PI(k), k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       const Expr* top {k.sub(x[1], N(k, "1"))};
                       return k.lor(k.lnot(k.land(k.ge_c(j, k.i32, "0"), k.lt(j, top))),
                                    k.le(brd(k, x[0], j), brd(k, x[0], k.add(j, N(k, "1")))));
                   });
    };
    // two pointers: forall same(d, s, n) = d[j] == s[j]
    [[maybe_unused]] auto same = [=](Kit& k, int id, const Expr* d, const Expr* sr, const Expr* n)
    {
        return mkq(k, id, "same", {d, sr, n}, {PI(k), PI(k), k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       return k.lor(k.lnot(in_range(k, j, x[2])), k.eq(brd(k, x[0], j), brd(k, x[1], j)));
                   });
    };
    // every element below `n` is non-negative: forall nonneg(a, n)
    [[maybe_unused]] auto nonneg = [=](Kit& k, int id, const Expr* a, const Expr* n)
    {
        return mkq(k, id, "nonneg", {a, n}, {PI(k), k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       return k.lor(k.lnot(in_range(k, j, x[1])), k.ge_c(brd(k, x[0], j), k.i32, "0"));
                   });
    };
    // forall kept_at(a, m) = a[j] == j + 1
    [[maybe_unused]] auto kept_at = [=](Kit& k, int id, const Expr* a, const Expr* n)
    {
        return mkq(k, id, "kept_at", {a, n}, {PI(k), k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       return k.lor(k.lnot(in_range(k, j, x[1])), k.eq(brd(k, x[0], j), k.add(j, N(k, "1"))));
                   });
    };

    // ---- the fill loop over an owned range (S45 (c) fill_range, must_init) ----
    // void fill(int *p, int n) [must_init p] requires(owns_range(p, n)) ensures(owns_range(p, n)) { int i = 0; while (i < n) inv 0 <= i <= n && forall is_idx(p, i) { p[i] = i; i++; } }
    // `mode`: 0 the plain fill (owns_range, the ensures says forall is_idx(p, n)), 1 the must_init filler, 2 the must_init filler of n - 1 elements, 3 the plain fill whose ensures claims forall is_idx(p, n) and
    // whose body writes i + 1
    const auto fill_range = [=](Kit& k, int mode, SrcLoc* end = nullptr) -> const Stmt*
    {
        Function* f {k.fn("fill", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        const bool marked {mode == 1 || mode == 2};
        if (marked)
        {
            f->params[0U].init = InitMarker::MustInit;
        }
        k.req(f, RO(k, f, 0, 1, 0));
        k.ens(f, marked ? RO(k, f, 0, 1, 0) : k.land(RO(k, f, 0, 1, 0), is_idx(k, 0, fx(k, f, 0), fx(k, f, 1))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* bound {mode == 2 ? k.sub(k.pv(f, 1), N(k, "1")) : k.pv(f, 1)};
        const Stmt* w {pwr(k, f, 0, k.v(i), mode == 3 ? k.add(k.v(i), N(k, "1")) : k.v(i))};
        const Stmt* lp {k.whl(k.lt(k.v(i), bound),
                              {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1)))),
                               k.cl(is_idx(k, 1, k.pn(f->param_var(0)), k.v(i)))},
                              {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({w, inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), lp});
        k.done(f);
        if (end != nullptr)
        {
            *end = f->end_loc;
        }
        return lp;
    };
    lcv("fql_fill_range",
        "the fill loop over owns_range(p, n): the invariant forall is_idx(p, i) frames p[j] by the range place, and the ensures forall is_idx(p, n) follows at i == n",
        [=](Kit& k)
        {
            fill_range(k, 0);
            vac_fn(k);
        });
    lcv("fql_must_init_filler",
        "a must_init filler: the quantified invariant's flag line carries the flags of the elements below i, and the flags row at the return holds (the verifying sibling of the pinned loop filler)",
        [=](Kit& k)
        {
            fill_range(k, 1);
            vac_fn(k);
        });
    lc("fql_must_init_filler_nminus1",
       "the filler stops at n - 1: the flags row at the return fails (postcondition at the closing brace)",
       [=](Kit& k)
       {
           SrcLoc end;
           fill_range(k, 2, &end);
           k.fail("postcondition", end, 1U, "", "returns without initializing every element");
       });

    // ---- the self-write loop (S45 (c)): int a[3] = {1, 2, 3}; while (i < 3) inv 0 <= i <= 3 && forall kept_at(a, 3) { a[i] = a[i]; i++; } return a[2]; ----
    // `mode`: 0 the loop, 1 the loop writes a[i] = 0 (the clause is not preserved), 2 the clause is stated for four elements (the framing and bound rows: a[3] is outside the array)
    const auto self_write = [=](Kit& k, int mode, Fill* info = nullptr)
    {
        Function* f {k.fn("f", k.i32)};
        k.ens(f, k.eq_c(k.R(f), k.i32, "3"));
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {kept_at(k, 0, k.addr(a), N(k, mode == 2 ? "4" : "3"))};
        const Clause user {k.cl(q)};
        const Stmt* w {awr(k, a, k.v(i), mode == 1 ? N(k, "0") : ard(k, a, k.v(i)))};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), user},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({w, inc(k, i)}))};
        f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                          awr(k, a, N(k, "2"), N(k, "3")), k.decl(i, N(k, "0")), lp, k.ret(ard(k, a, N(k, "2")))});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_self_write",
        "the loop stores a[i] = a[i], a write reaches the array so its values are not kept; the quantified invariant states them, and the ensures r == 3 follows",
        [=](Kit& k)
        {
            self_write(k, 0);
            vac_fn(k);
        });
    lc("fql_self_write_bad",
       "the loop stores a[i] = 0: the invariant a[j] == j + 1 is not preserved (invariant-preserved at the clause; the flag line holds)",
       [=](Kit& k)
       {
           Fill fi;
           self_write(k, 1, &fi);
           k.fail("invariant-preserved", fi.clause);
       });

    lc("fql_self_write_outside",
       "the clause is stated for four elements of a three-element array: the framing assert and the bound line report the slot outside the array",
       [=](Kit& k)
       {
           Fill fi;
           self_write(k, 2, &fi);
           // the framing asserted at entry fails (the one at the back edge is not reached: the bound line, assumed at the head, is false for the slot outside); the bound line fails at entry and at
           // the back edge; the flag line takes the bound in its antecedent and does not fail (audit round 1, N14)
           k.fail_only("permission", fi.q->loc, 2U, 0U);
           k.fail("out-of-bounds", fi.q->loc, 2U);
       });

    // ---- max and max_index over reads_range (S45 (c)) ----
    // int max(const int *a, int n) requires(reads_range(a, n) && n > 0) ensures(reads_range(a, n) && forall le_val(a, n, result))
    //   { int m = a[0]; int i = 1; while (i < n) inv 1 <= i <= n && forall le_val(a, i, m) decreases n - i { if (a[i] > m) m = a[i]; i++; } return m; }
    // `mode`: 0 max, 1 the invariant says `<` (it fails at entry: a[0] < a[0]), 2 the ensures says `<` (the postcondition fails), 3 the loop runs to i <= n (the read a[n] is outside the range)
    const auto max_fn = [=](Kit& k, int mode, Fill* info = nullptr, const Stmt** ret_stmt = nullptr)
    {
        Function* f {k.fn("max", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), le_val(k, 0, fx(k, f, 0), fx(k, f, 1), k.R(f), mode == 2)));
        Var* m {k.local(f, "m", k.i32)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {le_val(k, 1, k.pn(f->param_var(0)), k.v(i), k.v(m), mode == 1)};
        const Clause user {k.cl(q)};
        const Stmt* upd {k.iff(k.gt(prd(k, f, 0, k.v(i)), k.v(m)), k.asg(m, prd(k, f, 0, k.v(i))))};
        const Stmt* lp {k.whl(mode == 3 ? k.le(k.v(i), k.pv(f, 1)) : k.lt(k.v(i), k.pv(f, 1)),
                              {k.cl(k.land(k.ge_c(k.v(i), k.i32, "1"), k.le(k.v(i), k.pv(f, 1)))), user},
                              {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({upd, inc(k, i)}))};
        const Stmt* r {k.ret(k.v(m))};
        f->body = k.blk({k.decl(m, prd(k, f, 0, N(k, "0"))), k.decl(i, N(k, "1")), lp, r});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
        if (ret_stmt != nullptr)
        {
            *ret_stmt = r;
        }
    };
    lcv("fql_max",
        "max over reads_range(a, n): the invariant forall a[j] <= m for j < i, the ensures forall a[j] <= result",
        [=](Kit& k)
        {
            max_fn(k, 0);
            vac_fn(k);
        });
    lc("fql_max_strict", "the invariant says < for <=: it does not hold at entry (a[0] < a[0], invariant-entry)",
       [=](Kit& k)
       {
           Fill fi;
           max_fn(k, 1, &fi);
           k.fail("invariant-entry", fi.clause);
       });
    lc("fql_max_post_strict",
       "the ensures says every element is < the result: the return does not meet it (postcondition at the return)",
       [=](Kit& k)
       {
           const Stmt* r {nullptr};
           max_fn(k, 2, nullptr, &r);
           k.fail("postcondition", r->loc);
       });

    // int max_index(const int *a, int n) requires(reads_range(a, n) && n > 0) ensures(reads_range(a, n) && 0 <= result < n && forall le_at(a, n, result))
    //   { int j = 0; int i = 1; while (i < n) inv 1 <= i <= n && 0 <= j < i && forall le_at(a, i, j) decreases n - i { if (a[i] > a[j]) j = i; i++; } return j; }
    // `mode`: 0 the loop, 1 the body picks the smaller (the clause is not preserved), 2 the clause is stated for i + 1 elements (a[i] is read before the loop has seen it)
    const auto max_index = [=](Kit& k, int mode, Fill* info = nullptr)
    {
        Function* f {k.fn("max_index", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(k.land(RR(k, f, 0, 1, 0), k.land(k.ge_c(k.R(f), k.i32, "0"), k.lt(k.R(f), fx(k, f, 1)))),
                        le_at(k, 0, fx(k, f, 0), fx(k, f, 1), k.R(f))));
        Var* j {k.local(f, "j", k.i32)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {le_at(k, 1, k.pn(f->param_var(0)), mode == 2 ? k.add(k.v(i), N(k, "1")) : k.v(i), k.v(j))};
        const Clause user {k.cl(q)};
        const Stmt* upd {k.iff(mode == 1 ? k.lt(prd(k, f, 0, k.v(i)), prd(k, f, 0, k.v(j)))
                                         : k.gt(prd(k, f, 0, k.v(i)), prd(k, f, 0, k.v(j))),
                               k.asg(j, k.v(i)))};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 1)),
                              {k.cl(k.land(k.ge_c(k.v(i), k.i32, "1"), k.le(k.v(i), k.pv(f, 1)))),
                               k.cl(k.land(k.ge_c(k.v(j), k.i32, "0"), k.lt(k.v(j), k.v(i)))), user},
                              {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({upd, inc(k, i)}))};
        f->body = k.blk({k.decl(j, N(k, "0")), k.decl(i, N(k, "1")), lp, k.ret(k.v(j))});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_max_index",
        "max_index: a[j] <= a[r] for every j < i reads a[r], which the binder does not index: its framing, flag and bound rows are its own, under the quantifier's path; the ensures quantifies the same",
        [=](Kit& k)
        {
            max_index(k, 0);
            vac_fn(k);
        });
    lc("fql_max_index_smaller",
       "the body picks the smaller element: the clause is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           Fill fi;
           max_index(k, 1, &fi);
           k.fail("invariant-preserved", fi.clause);
       });
    // ---- an argument's own obligations (item 3, B1): once, before the quantifier, whether or not the body uses the parameter ----
    // void f(int k) { int a[3]; int i = 0; while (i < 3) inv 0 <= i <= 3 && forall is_idx_ignoring(a, i, [k | k + 1]) { a[i] = i; i++; } }
    const auto arg_loop = [=](Kit& k, bool overflow, Fill* info)
    {
        Function* f {k.fn("f", k.vd, {{"k", k.i32}})};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* extra {overflow ? k.add(k.pv(f, 0), N(k, "1")) : k.pv(f, 0)};
        const Expr* q {mkq(k, 0, "is_idx_ignoring", {k.addr(a), k.v(i), extra}, {PI(k), k.i32, k.i32},
                           [&](const Expr* j, const std::vector<const Expr*>& x)
                           {
                               return k.lor(k.lnot(in_range(k, j, x[1])), k.eq(brd(k, x[0], j), j));
                           })};
        const Clause user {k.cl(q)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), user},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({awr(k, a, k.v(i), k.v(i)), inc(k, i)}))};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")), lp});
        k.done(f);
        *info = Fill {q, user.loc};
        return extra;
    };
    lcv("fql_argument_rows",
        "a trailing argument k that the body does not use: no obligation of its own, the loop verifies",
        [=](Kit& k)
        {
            Fill fi;
            arg_loop(k, false, &fi);
            vac_fn(k);
        });
    lc("fql_argument_overflow",
       "the unused argument is k + 1: its overflow is an obligation line of its own before the quantifier, reported whether or not the body uses it (overflow at the argument)",
       [=](Kit& k)
       {
           Fill fi;
           const Expr* extra {arg_loop(k, true, &fi)};
           k.fail_only("overflow", extra->loc, 2U, 0U);
       });

    // ---- search (S45 (c)): r == -1 implies no element equals x ----
    // int search(const int *p, int n, int x) requires(reads_range(p, n)) ensures(reads_range(p, n) && (result != -1 || forall ne_val(p, n, x)))
    //   { int i = 0; while (i < n) inv 0 <= i <= n && forall ne_val(p, i, x) decreases n - i { if (p[i] == x) return i; i++; } return -1; }
    // `wrong`: the ensures claims every element EQUALS x when -1 is returned
    const auto search = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("search", k.i32, {{"p", PI(k)}, {"n", k.i32}, {"x", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        k.ens(f, k.land(RR(k, f, 0, 1, 0),
                        k.lor(k.ne(k.R(f), N(k, "-1")), ne_val(k, 0, fx(k, f, 0), fx(k, f, 1), fx(k, f, 2), wrong))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {ne_val(k, 1, k.pn(f->param_var(0)), k.v(i), k.pv(f, 2), false)};
        const Stmt* hit {k.ret(k.v(i))};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 1)),
                              {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1)))), k.cl(q)},
                              {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                              k.blkc({k.iff(k.eq(prd(k, f, 0, k.v(i)), k.pv(f, 2)), k.blkc({hit})), inc(k, i)}))};
        const Stmt* last {k.ret(N(k, "-1"))};
        f->body = k.blk({k.decl(i, N(k, "0")), lp, last});
        k.done(f);
        return last;
    };
    lcv("fql_search",
        "the search: forall p[j] != x for j < i as the invariant, and after the loop (r == -1) for j < n; the return inside the body needs no clause",
        [=](Kit& k)
        {
            search(k, false);
            vac_fn(k);
        });
    lc("fql_search_wrong",
       "the ensures claims every element equals x when -1 is returned: the postcondition fails at that return",
       [=](Kit& k)
       {
           k.fail("postcondition", search(k, true)->loc);
       });

    // ---- neighbours: two read classes of one array in a loop (S45 (b), branch T2) ----
    // int is_sorted(const int *a, int n) requires(reads_range(a, n) && n >= 1) ensures(reads_range(a, n) && (result == 0 || forall nondec(a, n)))
    //   { int i = 1; while (i < n) inv 1 <= i <= n && forall nondec(a, i) decreases n - i { if (a[i - 1] > a[i]) return 0; i++; } return 1; }
    // `bad`: the test is reversed (a[i - 1] < a[i] returns 0), so the clause is not preserved
    const auto sorted = [=](Kit& k, bool bad, Fill* info = nullptr)
    {
        Function* f {k.fn("is_sorted", k.i32, {{"a", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.lor(k.eq_c(k.R(f), k.i32, "0"), nondec(k, 0, fx(k, f, 0), fx(k, f, 1)))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {nondec(k, 1, k.pn(f->param_var(0)), k.v(i))};
        const Clause user {k.cl(q)};
        const Expr* prev {prd(k, f, 0, k.sub(k.v(i), N(k, "1")))};
        const Expr* cur {prd(k, f, 0, k.v(i))};
        const Stmt* lp {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.land(k.ge_c(k.v(i), k.i32, "1"), k.le(k.v(i), k.pv(f, 1)))), user},
                  {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                  k.blkc({k.iff(bad ? k.lt(prev, cur) : k.gt(prev, cur), k.blkc({k.ret(N(k, "0"))})), inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "1")), lp, k.ret(N(k, "1"))});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_sorted_loop",
        "a loop that checks sortedness: forall a[j] <= a[j + 1] for j < i - 1 (two read classes of one array, one binder each), the ensures says the whole array when 1 is returned",
        [=](Kit& k)
        {
            sorted(k, false);
            vac_fn(k);
        });
    lc("fql_sorted_loop_reversed",
       "the test is reversed: the clause is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           Fill fi;
           sorted(k, true, &fi);
           k.fail("invariant-preserved", fi.clause);
       });

    // ---- copy: two read classes of two pointers in a loop ----
    // void copy(int *dst, const int *src, int n) requires(owns_range(dst, n) && reads_range(src, n)) ensures(owns_range(dst, n) && reads_range(src, n) && forall same(dst, src, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n && forall same(dst, src, i) decreases n - i { dst[i] = src[i]; i++; } }
    // `zero`: the body stores 0 (the clause is not preserved)
    const auto copy = [=](Kit& k, bool zero, Fill* info = nullptr)
    {
        Function* f {k.fn("copy", k.vd, {{"dst", PI(k)}, {"src", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
        k.ens(f,
              k.land(k.land(RO(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)), same(k, 0, fx(k, f, 0), fx(k, f, 1), fx(k, f, 2))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {same(k, 1, k.pn(f->param_var(0)), k.pn(f->param_var(1)), k.v(i))};
        const Clause user {k.cl(q)};
        const Stmt* w {pwr(k, f, 0, k.v(i), zero ? N(k, "0") : prd(k, f, 1, k.v(i)))};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 2)),
                              {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 2)))), user},
                              {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, k.blkc({w, inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), lp});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_copy_loop",
        "dst[i] = src[i] over owns_range(dst, n) and reads_range(src, n): two classes (dst and src, each at its own binder) under one clause; the ensures states the same for n",
        [=](Kit& k)
        {
            copy(k, false);
            vac_fn(k);
        });
    lc("fql_copy_loop_zero",
       "the body stores 0 into dst[i]: the clause is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           Fill fi;
           copy(k, true, &fi);
           k.fail("invariant-preserved", fi.clause);
       });

    // ---- a `check` (item 10) ----
    auto pown = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, fx(k, f, i), amount, k.at());
    };
    // void f() { int a[4]; a[0] = 1; a[1] = 2; a[2] = 3; a[3] = 4; [eat(&a[1]);] check(forall nonneg(a, [4|5])); }
    // `mode`: 0 the check, 1 a[2] = -3 (the fact is false), 2 a[3] is not written (the flag row), 3 the range is five elements (the bound row), 4 a callee kept a[1] (the marked row)
    const auto check_local = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* eat {nullptr};
        if (mode == 4)
        {
            eat = k.fn("eat", k.vd, {{"p", PI(k)}});
            k.req(eat, pown(k, eat, 0, 0));
            k.trusted(eat);
        }
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int j {0}; j < 4; ++j)
        {
            if (mode == 2 && j == 3)
            {
                continue;
            }
            const std::string v {mode == 1 && j == 2 ? "-3" : std::to_string(j + 1)};
            st.push_back(awr(k, a, N(k, std::to_string(j).c_str()), N(k, v.c_str())));
        }
        if (eat != nullptr)
        {
            st.push_back(k.es(k.call(eat, {aat(k, a, N(k, "1"))})));
        }
        const Expr* q {nonneg(k, 0, k.addr(a), N(k, mode == 3 ? "5" : "4"))};
        const Stmt* c {k.chk(q)};
        st.push_back(c);
        f->body = k.blkc(std::move(st));
        k.done(f);
        return c;
    };
    lcv("fql_check_local",
        "check(forall nonneg(a, 4)) over a local array: the bound row, the marked rows for the value and the flag, the flag row, then the fact",
        [=](Kit& k)
        {
            check_local(k, 0);
            vac_fn(k);
        });
    lc("fql_check_local_false_fact", "an element is -3: every row holds, the fact fails (check)",
       [=](Kit& k)
       {
           k.fail("check", check_local(k, 1)->loc);
       });
    lc("fql_check_local_unwritten",
       "a[3] has no value: the flag row fails (uninitialized at the quantifier's site), and the path ends there",
       [=](Kit& k)
       {
           k.fail("uninitialized", check_local(k, 2)->expr->loc);
       });
    lc("fql_check_local_outside",
       "the range is five elements of a four-element array: the bound row fails (out-of-bounds at the site)",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", check_local(k, 3)->expr->loc, 1U, "", "outside `a`");
       });
    lc("fql_check_local_kept", "a callee kept a[1]: the marked row fails (permission at the site) and ends the path",
       [=](Kit& k)
       {
           // the value field's marked row, then the flag field's: the first failure ends the path, so only it is reported
           k.fail_only("permission", check_local(k, 4)->expr->loc, 2U, 0U);
       });

    // void f(int *p, int n) requires([owns_range|reads_range](p, n) [&& forall nonneg(p, n)]) ensures([owns_range|reads_range](p, n)) { check(forall nonneg(p, n)); }
    const auto check_param = [=](Kit& k, bool owns, bool fact) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        const Expr* atom_r {owns ? RO(k, f, 0, 1, 0) : RR(k, f, 0, 1, 0)};
        k.req(f, fact ? k.land(atom_r, nonneg(k, 0, fx(k, f, 0), fx(k, f, 1))) : atom_r);
        k.ens(f, owns ? RO(k, f, 0, 1, 0) : RR(k, f, 0, 1, 0));
        const Stmt* c {k.chk(nonneg(k, 1, k.pn(f->param_var(0)), k.pv(f, 1)))};
        f->body = k.blk({c});
        k.done(f);
        return c;
    };
    lcv("fql_check_owns_range",
        "check(forall nonneg(p, n)) over owns_range(p, n) with the fact in the requires: the null and liveness rows of a pointer base, the marked rows, the flag row, then the fact",
        [=](Kit& k)
        {
            check_param(k, true, true);
            vac_fn(k);
        });
    lc("fql_check_owns_range_nofact",
       "the same without the fact in the requires: every row holds, the fact fails (check)",
       [=](Kit& k)
       {
           k.fail("check", check_param(k, true, false)->loc);
       });
    lcv("fql_check_reads_range",
        "the same over reads_range(p, n) (a read amount): the marked wildcard rows hold for any positive amount",
        [=](Kit& k)
        {
            check_param(k, false, true);
            vac_fn(k);
        });
    lc("fql_check_reads_range_nofact", "reads_range without the fact: the fact fails (check)",
       [=](Kit& k)
       {
           k.fail("check", check_param(k, false, false)->loc);
       });

    // ---- two quantified invariants in one loop: the second's framing and obligation lines rest on the first (the prior) ----
    // void f() { int a[3]; int i = 0; while (i < 3) inv 0 <= i <= 3 && forall is_idx(a, i) && forall nonneg(a, i) { a[i] = i; i++; } check(a[2] >= 0 ...); }
    lcv("fql_two_quantifiers",
        "two quantified clauses of one loop: each has its own framing assert and obligation lines, the second's under the first as its prior",
        [=](Kit& k)
        {
            Function* f {k.fn("f", k.vd)};
            Var* a {arr(k, f, "a", k.i32, 3U)};
            Var* i {k.local(f, "i", k.i32)};
            const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                                  {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(is_idx(k, 0, k.addr(a), k.v(i))),
                                   k.cl(nonneg(k, 1, k.addr(a), k.v(i)))},
                                  {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({awr(k, a, k.v(i), k.v(i)), inc(k, i)}))};
            f->body =
                k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")), lp, k.chk(k.ge_c(ard(k, a, N(k, "2")), k.i32, "0"))});
            k.done(f);
            vac_fn(k);
        });

    // ---- a read the binder does not index, over a named array (max_index on a local array), and a check over neighbours ----
    // void f() { int a[3] = {1, 3, 2}; int j = 0; int i = 1; while (i < 3) inv 1 <= i <= 3 && 0 <= j < i && forall le_at(a, i, j) { if (a[i] > a[j]) j = i; i++; } check(forall le_at(a, 3, j)); }
    lcv("fql_max_index_local",
        "max_index over a local array: the read a[j] the binder does not index is framed by the array place, bounded and flagged under the quantifier's path; the check after the loop states a different fact (every element is non-negative)",
        [=](Kit& k)
        {
            Function* f {k.fn("f", k.vd)};
            Var* a {arr(k, f, "a", k.i32, 3U)};
            Var* j {k.local(f, "j", k.i32)};
            Var* i {k.local(f, "i", k.i32)};
            const Stmt* upd {k.iff(k.gt(ard(k, a, k.v(i)), ard(k, a, k.v(j))), k.asg(j, k.v(i)))};
            const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                                  {k.cl(k.land(k.ge_c(k.v(i), k.i32, "1"), k.le(k.v(i), N(k, "3")))),
                                   k.cl(k.land(k.ge_c(k.v(j), k.i32, "0"), k.lt(k.v(j), k.v(i)))),
                                   k.cl(le_at(k, 0, k.addr(a), k.v(i), k.v(j)))},
                                  {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({upd, inc(k, i)}))};
            f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "3")),
                              awr(k, a, N(k, "2"), N(k, "2")), k.decl(j, N(k, "0")), k.decl(i, N(k, "1")), lp,
                              k.chk(nonneg(k, 1, k.addr(a), N(k, "3")))});
            k.done(f);
            vac_fn(k);
        });
    // void f() { int a[3] = {1, 2, [3|1]}; check(forall nondec(a, 3)); }: two read classes of one array in a check (one set of rows per class)
    const auto check_nondec = [=](Kit& k, bool sorted_array) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Stmt* c {k.chk(nondec(k, 0, k.addr(a), N(k, "3")))};
        f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                          awr(k, a, N(k, "2"), N(k, sorted_array ? "3" : "1")), c});
        k.done(f);
        return c;
    };
    lcv("fql_check_neighbours",
        "check(forall a[j] <= a[j + 1]) over a local array: two read classes, a set of rows per class (the bound, the marked rows, the flag row), then the fact",
        [=](Kit& k)
        {
            check_nondec(k, true);
            vac_fn(k);
        });
    lc("fql_check_neighbours_false", "the last element is 1: the fact fails (check)",
       [=](Kit& k)
       {
           k.fail("check", check_nondec(k, false)->loc);
       });

    // ---- the refusals of check_program (item 3's bases, item 4's body, the sites where a quantifier does not stand) ----
    // a loop over a local array `a` and `i` with `inv` as its only clause, `meas` its measure, in the given form
    auto in_loop =
        [=](Kit& k, const std::function<const Expr*(Kit&, Function*, Var*, Var*)>& inv, LoopForm form, bool measure_q)
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {inv(k, f, a, i)};
        const Expr* cond {form == LoopForm::DoOnce ? k.no() : k.lt(k.v(i), N(k, "3"))};
        const Stmt* lp {k.loop(cond,
                               form == LoopForm::DoOnce ? std::vector<Clause> {k.cl(q)} : std::vector<Clause> {k.cl(q)},
                               {k.cl(measure_q ? k.b.bool_to_int(k.i32, q, k.at()) : k.sub(N(k, "3"), k.v(i)))},
                               k.blkc({inc(k, i)}), nullptr, form, k.jid())};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")), lp});
        k.done(f);
    };
    auto nonneg_of = [=](Kit& k, Function*, Var* a, Var* i)
    {
        return nonneg(k, 0, k.addr(a), k.v(i));
    };
    rc("fql_rej_in_measure", "a quantifier outside the `requires` and `ensures`",
       [=](Kit& k)
       {
           in_loop(k, nonneg_of, LoopForm::While, true);
       });
    rc("fql_rej_in_do_once", "a quantifier outside the `requires` and `ensures`",
       [=](Kit& k)
       {
           in_loop(k, nonneg_of, LoopForm::DoOnce, false);
       });
    rc("fql_rej_old_in_invariant", "`old` outside an `ensures`",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function*, Var* a, Var* i)
               {
                   return mkq(kk, 0, "kept", {kk.addr(a), kk.v(i)}, {PI(kk), kk.i32},
                              [&](const Expr* j, const std::vector<const Expr*>& x)
                              {
                                  return kk.lor(kk.lnot(in_range(kk, j, x[1])),
                                                kk.eq(brd(kk, x[0], j), kk.old(brd(kk, x[0], j))));
                              });
               },
               LoopForm::While, false);
       });
    rc("fql_rej_base_element_address", "is not a pointer variable or a named local array",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function*, Var* a, Var* i)
               {
                   return nonneg(kk, 0, aat(kk, a, N(kk, "1")), kk.v(i));
               },
               LoopForm::While, false);
       });
    rc("fql_rej_base_address_taken_pointer", "reads memory",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function* ff, Var*, Var* i)
               {
                   Var* p {kk.cell(ff, "p", PI(kk))};
                   return nonneg(kk, 0, kk.v(p), kk.v(i));
               },
               LoopForm::While, false);
       });
    rc("fql_rej_deref_of_array_argument", "`*p` in the body of a quantifier of an array argument",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function*, Var* a, Var* i)
               {
                   return mkq(kk, 0, "first_is", {kk.addr(a), kk.v(i)}, {PI(kk), kk.i32},
                              [&](const Expr* j, const std::vector<const Expr*>& x)
                              {
                                  return kk.lor(
                                      kk.lnot(in_range(kk, j, x[1])),
                                      kk.land(kk.eq(brd(kk, x[0], j), j), kk.ge_c(kk.deref(x[0]), kk.i32, "0")));
                              });
               },
               LoopForm::While, false);
       });
    rc("fql_rej_pointer_as_value", "a pointer argument used as a value",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function*, Var* a, Var* i)
               {
                   return mkq(kk, 0, "not_null", {kk.addr(a), kk.v(i)}, {PI(kk), kk.i32},
                              [&](const Expr* j, const std::vector<const Expr*>& x)
                              {
                                  return kk.lor(kk.lnot(in_range(kk, j, x[1])),
                                                kk.land(kk.eq(brd(kk, x[0], j), j), kk.ne(x[0], kk.nul(PI(kk)))));
                              });
               },
               LoopForm::While, false);
       });
    // a `check` over a read the bound variable does not index, and over pointer elements
    rc("fql_rej_check_unindexed_read", "a read the bound variable does not index, in a quantifier of a `check`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           Var* j {k.local(f, "j", k.i32)};
           f->body =
               k.blk({k.decl(a, nullptr), k.decl(j, N(k, "0")), k.chk(le_at(k, 0, k.addr(a), N(k, "3"), k.v(j)))});
           k.done(f);
       });
    rc("fql_rej_check_pointer_elements", "over pointer elements",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", PI(k), 3U)};
           const Expr* q {mkq(k, 0, "not_null_at", {k.addr(a), N(k, "3")}, {k.ptr(PI(k)), k.i32},
                              [&](const Expr* j, const std::vector<const Expr*>& x)
                              {
                                  return k.lor(k.lnot(in_range(k, j, x[1])), k.ne(brd(k, x[0], j), k.nul(PI(k))));
                              })};
           f->body = k.blk({k.decl(a, nullptr), k.chk(q)});
           k.done(f);
       });
    rc("fql_rej_check_old", "`old` outside an `ensures`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           const Expr* q {mkq(k, 0, "kept", {k.addr(a), N(k, "3")}, {PI(k), k.i32},
                              [&](const Expr* j, const std::vector<const Expr*>& x)
                              {
                                  return k.lor(k.lnot(in_range(k, j, x[1])),
                                               k.eq(brd(k, x[0], j), k.old(brd(k, x[0], j))));
                              })};
           f->body = k.blk({k.decl(a, nullptr), k.chk(q)});
           k.done(f);
       });

    // ---- a loop that only counts, whose invariant states a fact about the range it reads: the quantified read makes the loop carry the range (expr_touches) ----
    // int f(const int *p, int n) requires(reads_range(p, n) && n > 0 && forall nonneg(p, n)) ensures(reads_range(p, n) && result >= 0)
    //   { int i = 0; while (i < n) inv 0 <= i <= n && forall nonneg(p, n) decreases n - i { i++; } return p[0]; }
    // `drop`: the requires has no quantified fact (the invariant does not hold at entry)
    const auto read_only = [=](Kit& k, bool drop, Fill* info = nullptr)
    {
        Function* f {k.fn("f", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        const Expr* base {k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0")))};
        k.req(f, drop ? base : k.land(base, nonneg(k, 0, fx(k, f, 0), fx(k, f, 1))));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(k.R(f), k.i32, "0")));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {nonneg(k, 1, k.pn(f->param_var(0)), k.pv(f, 1))};
        const Clause user {k.cl(q)};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 1)),
                              {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1)))), user},
                              {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), lp, k.ret(prd(k, f, 0, N(k, "0")))});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_read_only_loop",
        "a loop that only counts, with forall p[j] >= 0 for the whole range as its invariant: the fact holds at entry (the requires), is kept (no write reaches), and p[0] >= 0 after the loop",
        [=](Kit& k)
        {
            read_only(k, false);
            vac_fn(k);
        });
    lc("fql_read_only_loop_nofact",
       "the requires does not state the fact: the invariant does not hold on entry (invariant-entry at the clause)",
       [=](Kit& k)
       {
           Fill fi;
           read_only(k, true, &fi);
           k.fail("invariant-entry", fi.clause);
       });

    // void f(int *p, int n) requires(owns_range(p, n) && forall nonneg(p, n)) ensures(owns_range(p, n)) { int i = 0; while (i < n) inv 0 <= i <= n && forall nonneg(p, n) decreases n - i { p[i] = [0|-1]; i++; } }
    // the body writes the range the invariant speaks about: a write reaches the field, so the values are not kept, and the clause itself must be preserved
    const auto write_range = [=](Kit& k, bool breaks, Fill* info = nullptr)
    {
        Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), nonneg(k, 0, fx(k, f, 0), fx(k, f, 1))));
        k.ens(f, RO(k, f, 0, 1, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* q {nonneg(k, 1, k.pn(f->param_var(0)), k.pv(f, 1))};
        const Clause user {k.cl(q)};
        const Stmt* lp {k.whl(
            k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1)))), user},
            {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({pwr(k, f, 0, k.v(i), N(k, breaks ? "-1" : "0")), inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), lp});
        k.done(f);
        if (info != nullptr)
        {
            *info = Fill {q, user.loc};
        }
    };
    lcv("fql_write_keeps_fact",
        "the body writes 0 into every element: the fact forall p[j] >= 0 is restated by the invariant and preserved by the write",
        [=](Kit& k)
        {
            write_range(k, false);
            vac_fn(k);
        });
    lc("fql_write_breaks_fact",
       "the body writes -1: a write reaches the field, the old fact is not kept, and the clause is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           Fill fi;
           write_range(k, true, &fi);
           k.fail("invariant-preserved", fi.clause);
       });

    // ---- audit round 3 ----
    // question 261: arithmetic over the read of a class other than the first, in a quantifier with several classes, in a loop invariant or a check
    auto plus_nondec = [=](Kit& k, int id, const Expr* a, const Expr* n, bool first)
    {
        // forall(j) !(0 <= j && j < n - 1) || (first ? a[j] + 1 <= a[j + 1] : a[j] <= a[j + 1] + 1)
        return mkq(k, id, "plus_nondec", {a, n}, {PI(k), k.i32},
                   [&](const Expr* j, const std::vector<const Expr*>& x)
                   {
                       const Expr* top {k.sub(x[1], N(k, "1"))};
                       const Expr* lo {brd(k, x[0], j)};
                       const Expr* hi {brd(k, x[0], k.add(j, N(k, "1")))};
                       return k.lor(k.lnot(k.land(k.ge_c(j, k.i32, "0"), k.lt(j, top))),
                                    first ? k.le(k.add(lo, N(k, "1")), hi) : k.le(lo, k.add(hi, N(k, "1"))));
                   });
    };
    rc("fql_rej_arith_other_class_check", "a class other than the first",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           f->body = k.blk({k.decl(a, nullptr), k.chk(plus_nondec(k, 0, k.addr(a), N(k, "3"), false))});
           k.done(f);
       });
    rc("fql_rej_arith_other_class_loop", "a class other than the first",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function*, Var* a, Var* i)
               {
                   return plus_nondec(kk, 0, kk.addr(a), kk.v(i), false);
               },
               LoopForm::While, false);
       });
    lcv("fql_check_arith_first_class",
        "arithmetic over the first class's read alone (a[j] + 1 <= a[j + 1]): its overflow row is the first class's, and the strictly increasing array verifies",
        [=](Kit& k)
        {
            Function* f {k.fn("f", k.vd)};
            Var* a {arr(k, f, "a", k.i32, 3U)};
            f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                              awr(k, a, N(k, "2"), N(k, "3")), k.chk(plus_nondec(k, 0, k.addr(a), N(k, "3"), true))});
            k.done(f);
            vac_fn(k);
        });
    // question 262: a check that holds a quantifier is one assert per top-level conjunct; `||` and `?:` positions are refused
    // void f() { int a[4] = {1, 2, 3, 4 | 2, ...}; check([a[0] == 1 && forall nonneg(a, 4) | forall nonneg(a, 4) && a[0] == 1]); }
    const auto check_and = [=](Kit& k, bool forall_first, int first_value, const Stmt** out_check = nullptr)
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int j {0}; j < 4; ++j)
        {
            const std::string v {std::to_string(j == 0 ? first_value : j + 1)};
            st.push_back(awr(k, a, N(k, std::to_string(j).c_str()), N(k, v.c_str())));
        }
        const Expr* scalar {k.eq_c(ard(k, a, N(k, "0")), k.i32, "1")};
        const Expr* q {nonneg(k, 0, k.addr(a), N(k, "4"))};
        const Stmt* c {k.chk(forall_first ? k.land(q, scalar) : k.land(scalar, q))};
        st.push_back(c);
        f->body = k.blkc(std::move(st));
        k.done(f);
        if (out_check != nullptr)
        {
            *out_check = c;
        }
    };
    lcv("fql_check_and_last",
        "check(a[0] == 1 && forall nonneg(a, 4)): one assert per top-level conjunct, the quantifier's own rows and fact in a line of their own",
        [=](Kit& k)
        {
            check_and(k, false, 1);
            vac_fn(k);
        });
    lcv("fql_check_and_first", "check(forall nonneg(a, 4) && a[0] == 1): the quantifier first",
        [=](Kit& k)
        {
            check_and(k, true, 1);
            vac_fn(k);
        });
    lc("fql_check_and_false_conjunct",
       "a[0] is 2: the scalar conjunct fails (check, one error at the check's line), the quantifier's assert holds",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           check_and(k, false, 2, &c);
           k.fail_only("check", c->loc, 2U, 0U);
       });
    lc("fql_check_and_false_conjunct_last",
       "the same with the quantifier first: the scalar conjunct is the second assert (check, one error)",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           check_and(k, true, 2, &c);
           k.fail_only("check", c->loc, 2U, 1U);
       });
    rc("fql_rej_check_under_or", "not a top-level conjunct",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"g", k.bl}})};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           f->body = k.blk({k.decl(a, nullptr), k.chk(k.lor(k.pv(f, 0), nonneg(k, 0, k.addr(a), N(k, "3"))))});
           k.done(f);
       });
    rc("fql_rej_check_in_cond_arm", "not a top-level conjunct",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"g", k.bl}})};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           f->body =
               k.blk({k.decl(a, nullptr), k.chk(k.cond(k.pv(f, 0), nonneg(k, 0, k.addr(a), N(k, "3")), k.yes()))});
           k.done(f);
       });
    rc("fql_rej_invariant_under_or", "not a top-level conjunct",
       [=](Kit& k)
       {
           in_loop(
               k,
               [=](Kit& kk, Function*, Var* a, Var* i)
               {
                   return kk.lor(kk.eq_c(kk.v(i), kk.i32, "0"), nonneg(kk, 0, kk.addr(a), kk.v(i)));
               },
               LoopForm::While, false);
       });
    // note 7: a read no place of the loop holds is reported once, as `permission` at the framing assert, not also as `uninitialized` through the dummy `false`
    // void f(int *p, int n) { int i = 0; while (i < n) inv 0 <= i <= n && forall nonneg(p, i) decreases n - i { i++; } }
    lc("fql_unframed_read",
       "the invariant reads p[j] of a pointer no place of the loop holds: one permission error at the framing assert (the back edge: the range below i = 0 is empty at entry), no uninitialized error through the chain's dummy",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.ge_c(fx(k, f, 1), k.i32, "0"));
           Var* i {k.local(f, "i", k.i32)};
           const Expr* q {nonneg(k, 0, k.pn(f->param_var(0)), k.v(i))};
           const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 1)),
                                 {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1)))), k.cl(q)},
                                 {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({inc(k, i)}))};
           f->body = k.blk({k.decl(i, N(k, "0")), lp});
           k.done(f);
           // the framing asserted at entry holds (the range below i = 0 is empty); the one at the back edge fails
           k.fail_only("permission", q->loc, 2U, 1U);
       });
}

} // namespace
