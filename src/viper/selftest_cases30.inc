// (included by selftest.cpp) Ranges in contracts, M4.5e1b Part B (record 0060 items 3, 4, 5, 7 and 9): `owns_range(p, n)` and `reads_range(p, n)` in a function's `requires`
// and `ensures` (the pointer a parameter, the count an integer expression that reads no memory), the element reads `p[e]` of a pointer parameter in a contract, the
// callee's side of them.  A range atom lowers to one quantifier per fact over the absolute slot (`forall k :: { loc(block_of(p), k) } offset_of(p) <= k < offset_of(p) + n
// ==> ...`), a guard outside each, one conjunct per `requires` and `ensures` line; its `f__wf` is the pure line; a read `p[e]` is framed by a range atom's interval (or a cell
// atom's cell) and has one ghost value per base, index and state; at a return the marked rows (value then flag, the sum form where two read atoms may overlap) stand inside
// `if (g) { }`, then the shape row, the flags, the type's range and the liveness sum.
// A caller of a range function (Part C) and a loop in one (Part D) are still refused: the last cases are the backstops.
// Each rule has a case that verifies and a twin that fails for one reason (invariant 2); a failure's category and the VIR node whose position the record gives are the
// expectation (the store or the read for a body access, the contract's read for a framing failure, the closing brace or the `return` for a return row).  The twins that need a
// call (a callee that keeps the range) are Part C's: no call is made here.
namespace {

void define_ranges_lowering()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // `owns_range(p, n)` and `reads_range(p, n)` over parameters `p` and `n` of `f`, the ghost amount `amount`
    auto RO = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    // `p[e]` as a read in a contract (the Deref of an element address of the parameter), and in a body (of the parameter's local copy)
    auto crd = [=](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.deref(k.b.elem_addr(fx(k, f, p), idx, k.at()));
    };
    auto bat = [](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.b.elem_addr(k.pn(f->param_var(p)), idx, k.at());
    };
    auto brd = [=](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.deref(bat(k, f, p, idx));
    };
    auto bwr = [=](Kit& k, Function* f, int p, const Expr* idx, const Expr* v)
    {
        return k.store(bat(k, f, p, idx), v);
    };
    auto ge0 = [=](Kit& k, Function* f, int i)
    {
        return k.ge_c(fx(k, f, i), k.i32, "0");
    };
    auto lt = [=](Kit& k, Function* f, int i, int n)
    {
        return k.lt(fx(k, f, i), fx(k, f, n));
    };
    // a check that must fail (the vacuity twin: the path the contract describes is not empty)
    auto vacuity = [=](Kit& k, const Stmt* c)
    {
        k.fail("check", c->loc);
    };
    // the body of a function that does nothing, or (the vacuity twin) one `check(false)`, which is returned
    auto idle = [=](Kit& k, Function* f, bool vac) -> const Stmt*
    {
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        f->body = k.blk(c != nullptr ? std::vector<const Stmt*> {c} : std::vector<const Stmt*> {});
        k.done(f);
        return c;
    };

    // ---- items 3 and 4: the atom, a write and a read through it ----
    // int gs(int *p, int n, int i, int v) requires(owns_range(p, n) && 0 <= i && i < n && 0 <= v && v <= 100) ensures(owns_range(p, n) && result == [v|v + 1]) { p[i] = v; return p[i]; }
    const auto get_set = [=](Kit& k, bool wrong, bool vac) -> const Stmt*
    {
        Function* f {k.fn("gs", k.i32, {{"p", PI(k)}, {"n", k.i32}, {"i", k.i32}, {"v", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.land(k.land(ge0(k, f, 2), lt(k, f, 2, 1)),
                                                  k.land(ge0(k, f, 3), k.le_c(fx(k, f, 3), k.i32, "100")))));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), k.eq(k.R(f), wrong ? k.add(fx(k, f, 3), N(k, "1")) : fx(k, f, 3))));
        const Stmt* r {k.ret(brd(k, f, 0, k.pv(f, 2)))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {bwr(k, f, 0, k.pv(f, 2), k.pv(f, 3))};
        if (c != nullptr)
        {
            body.push_back(c);
        }
        body.push_back(r);
        f->body = k.blk(std::move(body));
        k.done(f);
        return vac ? c : r;
    };
    lc("rng_get_set", "p[i] = v; return p[i] under owns_range(p, n) and 0 <= i < n: the bound, permission and flag rows hold, the range comes back",
       [=](Kit& k)
       {
           get_set(k, false, false);
       });
    lc("rng_get_set_wrong", "the same claiming result == v + 1: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", get_set(k, true, false)->loc);
       });
    lc("rng_get_set_vacuity", "the same with check(false) before the return: it fails (the path through the contract is not empty)",
       [=](Kit& k)
       {
           vacuity(k, get_set(k, false, true));
       });
    // void st(int *p, int n, int i) requires(owns_range(p, n) && 0 <= i && i <= [n - 1|n]) ensures(owns_range(p, n)) { p[i] = 0; }
    const auto store_at = [=](Kit& k, bool past) -> const Stmt*
    {
        Function* f {k.fn("st", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"i", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0),
                        k.land(ge0(k, f, 2), past ? k.le(fx(k, f, 2), fx(k, f, 1)) : lt(k, f, 2, 1))));
        k.ens(f, RO(k, f, 0, 1, 0));
        const Stmt* w {bwr(k, f, 0, k.pv(f, 2), N(k, "0"))};
        f->body = k.blk({w});
        k.done(f);
        return w;
    };
    lc("rng_store_in", "p[i] = 0 for 0 <= i < n under owns_range(p, n)",
       [=](Kit& k)
       {
           store_at(k, false);
       });
    lc("rng_store_past",
       "the same with i <= n: p[n] is outside the range: the bound row, then the permission row (the failed bound is assumed and the slot is not held: two errors at the store, N1)",
       [=](Kit& k)
       {
           const Stmt* w {store_at(k, true)};
           k.fail("out-of-bounds", w->loc);
           k.fail("permission", w->loc);
       });

    // ---- reads_range, a contract read in an ensures ----
    // int rd(int *p, int n, int i) requires(reads_range(p, n) && 0 <= i && i < n) ensures(reads_range(p, n) && result == p[i]) { [return p[i];|p[i] = 0; return 0;] }
    const auto read_at = [=](Kit& k, bool write, bool vac) -> const Stmt*
    {
        Function* f {k.fn("rd", k.i32, {{"p", PI(k)}, {"n", k.i32}, {"i", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.land(ge0(k, f, 2), lt(k, f, 2, 1))));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.eq(k.R(f), crd(k, f, 0, fx(k, f, 2)))));
        const Stmt* w {write ? bwr(k, f, 0, k.pv(f, 2), N(k, "0")) : nullptr};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        const Stmt* r {k.ret(write ? N(k, "0") : brd(k, f, 0, k.pv(f, 2)))};
        std::vector<const Stmt*> body;
        for (const Stmt* s : {w, c, r})
        {
            if (s != nullptr)
            {
                body.push_back(s);
            }
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return vac ? c : (write ? w : r);
    };
    lc("rng_reads_read", "return p[i] under reads_range(p, n), the ensures reads p[i] too: the read is framed by the range, the body's read equals it",
       [=](Kit& k)
       {
           read_at(k, false, false);
       });
    lc("rng_reads_read_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, read_at(k, false, true));
       });
    lc("rng_reads_write", "the same body writes p[i] = 0: reads_range holds a fraction only, the write permission row fails (permission at the store)",
       [=](Kit& k)
       {
           k.fail("permission", read_at(k, true, false)->loc);
       });

    // ---- guards and the empty range ----
    // void sg(int *p, int n) requires(0 <= n && (n == 0 || owns_range(p, n))) ensures(n == 0 || owns_range(p, n)) { [if (n != 0) {] p[0] = 7; [}] }
    const auto guarded = [=](Kit& k, bool unguarded, bool vac) -> const Stmt*
    {
        Function* f {k.fn("sg", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(ge0(k, f, 1), k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0))));
        k.ens(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), N(k, "7"))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        const Stmt* inner {unguarded ? w : k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blk({w}), nullptr)};
        // the vacuity twin stands under the guard (audit round 2, S3): reached on the guard-true path only
        const Stmt* tail {c != nullptr ? k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blk({c}), nullptr) : nullptr};
        f->body = k.blk(tail != nullptr ? std::vector<const Stmt*> {inner, tail} : std::vector<const Stmt*> {inner});
        k.done(f);
        return vac ? c : w;
    };
    lc("rng_guard_n", "under n == 0 || owns_range(p, n) the write p[0] = 7 inside if (n != 0): the guard stands outside every quantifier, the marked rows inside if (g)",
       [=](Kit& k)
       {
           guarded(k, false, false);
       });
    lc("rng_guard_n_vacuity", "the same with check(false) under if (n != 0) after the write: it fails (the guard-true path is not empty)",
       [=](Kit& k)
       {
           vacuity(k, guarded(k, false, true));
       });
    lc("rng_guard_n_unguarded",
       "the same writing p[0] without testing n: p may be null and nothing is held when n == 0: the null row and the lifetime row fail (two errors at the store)",
       [=](Kit& k)
       {
           const Stmt* w {guarded(k, true, false)};
           k.fail("null-dereference", w->loc);
           k.fail_only("lifetime", w->loc, 1U, 0U);
       });
    // void sp(int *p, int n) requires(p == NULL || owns_range(p, n)) ensures(p == NULL || owns_range(p, n)) { if (p != NULL) ... }  (a guard on the pointer)
    const auto guard_p = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("sp", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(ge0(k, f, 1), k.lor(k.eq(fx(k, f, 0), k.nul(PI(k))), RO(k, f, 0, 1, 0))));
        k.ens(f, k.lor(k.eq(fx(k, f, 0), k.nul(PI(k))), RO(k, f, 0, 1, 0)));
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        f->body = k.blk(c != nullptr ? std::vector<const Stmt*> {k.iff(k.ne(k.pv(f, 0), k.nul(PI(k))), k.blk({c}), nullptr)}
                                     : std::vector<const Stmt*> {});
        k.done(f);
        return c;
    };
    lc("rng_guard_p", "under p == NULL || owns_range(p, n) and 0 <= n: nothing to do when p is null; the body does nothing, the return rows hold under the guard",
       [=](Kit& k)
       {
           guard_p(k, false);
       });
    lc("rng_guard_p_vacuity", "the same with check(false) under if (p != NULL): it fails (the guard-true path is not empty)",
       [=](Kit& k)
       {
           vacuity(k, guard_p(k, true));
       });
    // void er(int *p) requires(owns_range(p, 0)) ensures(owns_range(p, 0)) { [|int x = p[0];] }
    const auto empty_range = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("er", k.vd, {{"p", PI(k)}})};
        k.req(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), N(k, "0"), 0, k.at()));
        k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), N(k, "0"), 0, k.at()));
        if (mode == 1)
        {
            const Expr* r {brd(k, f, 0, N(k, "0"))};
            Var* x {k.local(f, "x", k.i32)};
            const Stmt* d {k.decl(x, r)};
            f->body = k.blk({d});
            k.done(f);
            return d;
        }
        return idle(k, f, mode == 2);
    };
    lc("rng_empty", "owns_range(p, 0): the liveness fraction alone, p non-null, nothing else; the function returns it",
       [=](Kit& k)
       {
           empty_range(k, 0);
       });
    lc("rng_empty_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, empty_range(k, 2));
       });
    lc("rng_empty_read",
       "the same reading p[0]: the bound row may fail (the object may end at p) and no cell is held: two errors at the read",
       [=](Kit& k)
       {
           const Stmt* d {empty_range(k, 1)};
           k.fail("out-of-bounds", d->expr->loc);
           k.fail("permission", d->expr->loc);
       });

    // ---- element types: bool, an enumeration, a pointer (WF in place of a range) ----
    // void sb(bool *p, int n) requires(owns_range(p, n) && 1 <= n) ensures(owns_range(p, n)) { p[0] = true; }
    const auto bool_elements = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("sb", k.vd, {{"p", k.ptr(k.bl)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
        k.ens(f, RO(k, f, 0, 1, 0));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), k.yes())};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        f->body = k.blk(c != nullptr ? std::vector<const Stmt*> {w, c} : std::vector<const Stmt*> {w});
        k.done(f);
        return c;
    };
    lc("rng_bool_elements", "owns_range over bool elements: no range row, the flag and the permission rows only",
       [=](Kit& k)
       {
           bool_elements(k, false);
       });
    lc("rng_bool_elements_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, bool_elements(k, true));
       });
    // enum E { 0..2 }; void se(enum E *p, int n) requires(owns_range(p, n) && 1 <= n) ensures(owns_range(p, n)) { p[0] = 1; }
    const auto enum_elements = [=](Kit& k, bool vac) -> const Stmt*
    {
        const Type e {k.en("E", 32, true, "0", "2")};
        Function* f {k.fn("se", k.vd, {{"p", k.ptr(e)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
        k.ens(f, RO(k, f, 0, 1, 0));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), k.num(e, "1"))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        f->body = k.blk(c != nullptr ? std::vector<const Stmt*> {w, c} : std::vector<const Stmt*> {w});
        k.done(f);
        return c;
    };
    lc("rng_enum_elements", "owns_range over an enumeration's elements: the type's range is the enumeration's [0, 2], stored value 1 is in it",
       [=](Kit& k)
       {
           enum_elements(k, false);
       });
    lc("rng_enum_elements_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, enum_elements(k, true));
       });
    // void sq(int **p, int n) requires(owns_range(p, n) && 1 <= n) ensures(owns_range(p, n)) { p[0] = NULL; }
    const auto ptr_elements = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("sq", k.vd, {{"p", k.ptr(PI(k))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
        k.ens(f, RO(k, f, 0, 1, 0));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), k.nul(PI(k)))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        f->body = k.blk(c != nullptr ? std::vector<const Stmt*> {w, c} : std::vector<const Stmt*> {w});
        k.done(f);
        return c;
    };
    lc("rng_ptr_elements", "owns_range over pointer elements: WF of the stored pointer in place of the range, a null element is well-formed",
       [=](Kit& k)
       {
           ptr_elements(k, false);
       });
    lc("rng_ptr_elements_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, ptr_elements(k, true));
       });

    // ---- item 5: element reads in contracts, their framing and f__wf ----
    // void c1(int *p) requires(owns(*p) && p[0] > 0) ensures(owns(*p)) {}    and requires(owns(*p) && p[1] > 0): the cell of p[1] is not the cell p owns
    const auto cell_frame = [=](Kit& k, const char* idx, const Stmt** vc = nullptr) -> const Expr*
    {
        Function* f {k.fn("c1", k.vd, {{"p", PI(k)}})};
        const Expr* r {crd(k, f, 0, N(k, idx))};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()), k.gt(r, N(k, "0"))));
        k.ens(f, k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()));
        const Stmt* c {idle(k, f, vc != nullptr)};
        if (vc != nullptr)
        {
            *vc = c;
        }
        return r;
    };
    lc("rng_read_by_cell", "requires(owns(*p) && p[0] > 0): the element read is framed by the cell p owns (the address of p[0] is p)",
       [=](Kit& k)
       {
           cell_frame(k, "0");
       });
    lc("rng_read_by_cell_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           cell_frame(k, "0", &c);
           vacuity(k, c);
       });
    lc("rng_read_by_cell_other",
       "the same reading p[1]: the cell atom frames only p's own cell: permission at the contract read (f__wf)",
       [=](Kit& k)
       {
           k.fail("permission", cell_frame(k, "1")->loc);
       });
    // int ro(int *p, int n, int i) requires(reads_range(p, n) && 0 <= i && i < [n|n + 1]) ensures(reads_range(p, n)) { return 0; }  with the read p[i] in the requires: p[n] is outside the interval
    const auto frame_bound = [=](Kit& k, bool past, const Stmt** vc = nullptr) -> const Expr*
    {
        Function* f {k.fn("fb", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"i", k.i32}})};
        const Expr* r {crd(k, f, 0, fx(k, f, 2))};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.land(k.land(ge0(k, f, 2), past ? k.le(fx(k, f, 2), fx(k, f, 1)) : lt(k, f, 2, 1)),
                                                  k.ge_c(r, k.i32, "0"))));
        k.ens(f, RR(k, f, 0, 1, 0));
        const Stmt* c {idle(k, f, vc != nullptr)};
        if (vc != nullptr)
        {
            *vc = c;
        }
        return r;
    };
    lc("rng_read_in_range", "requires(reads_range(p, n) && 0 <= i < n && p[i] >= 0): the read is inside the interval, its framing assertion holds",
       [=](Kit& k)
       {
           frame_bound(k, false);
       });
    lc("rng_read_in_range_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           frame_bound(k, false, &c);
           vacuity(k, c);
       });
    lc("rng_read_past_range",
       "the same with i <= n: p[n] is outside the interval: permission at the read, and no separate bound row",
       [=](Kit& k)
       {
           k.fail("permission", frame_bound(k, true)->loc);
       });
    // void nr(int *p) requires(p[0] > 0) {}  no atom at all
    lc("rng_read_no_atom", "requires(p[0] > 0) with no atom: the framing disjunction is empty: permission at the read",
       [=](Kit& k)
       {
           Function* f {k.fn("nr", k.vd, {{"p", PI(k)}})};
           const Expr* r {crd(k, f, 0, N(k, "0"))};
           k.req(f, k.gt(r, N(k, "0")));
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", r->loc);
       });
    // void up(int *p, int n, int i, int j) requires(owns_range(p, n) && 0 <= i < n && 0 <= j < n [&& j != i]) ensures(owns_range(p, n) && p[j] == old(p[j])) { p[i] = 0; }
    const auto keep_other = [=](Kit& k, bool with_distinct, const Stmt** vc = nullptr) -> Function*
    {
        Function* f {k.fn("up", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"i", k.i32}, {"j", k.i32}})};
        const Expr* bounds {k.land(k.land(ge0(k, f, 2), lt(k, f, 2, 1)), k.land(ge0(k, f, 3), lt(k, f, 3, 1)))};
        k.req(f, k.land(RO(k, f, 0, 1, 0), with_distinct ? k.land(bounds, k.ne(fx(k, f, 2), fx(k, f, 3))) : bounds));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), k.eq(crd(k, f, 0, fx(k, f, 3)), k.old(crd(k, f, 0, fx(k, f, 3))))));
        const Stmt* c {vc != nullptr ? k.chk(k.no()) : nullptr};
        const Stmt* w {bwr(k, f, 0, k.pv(f, 2), N(k, "0"))};
        f->body = k.blk(c != nullptr ? std::vector<const Stmt*> {w, c} : std::vector<const Stmt*> {w});
        k.done(f);
        if (vc != nullptr)
        {
            *vc = c;
        }
        return f;
    };
    lc("rng_old_element", "p[i] = 0 keeps p[j] for j != i: ensures p[j] == old(p[j]), two reads of one index with their ghosts (pre and post) and the equal-address facts",
       [=](Kit& k)
       {
           keep_other(k, true);
       });
    lc("rng_old_element_vacuity", "the same with check(false) after the write: it fails",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           keep_other(k, true, &c);
           vacuity(k, c);
       });
    lc("rng_old_element_alias", "the same without j != i: i may be j: postcondition at the closing brace",
       [=](Kit& k)
       {
           k.fail("postcondition", keep_other(k, false)->end_loc);
       });
    // int ds(int *p, int n) requires(owns_range(p, n) && 1 <= n) ensures(owns_range(p, n)) decreases(p[0]) { return 0; }  (an element in a measure, framed by the requires)
    const auto dec_element = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("ds", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
        k.req(f, k.ge(crd(k, f, 0, N(k, "0")), N(k, "0")));
        k.ens(f, RO(k, f, 0, 1, 0));
        k.dec(f, crd(k, f, 0, N(k, "0")));
        return idle(k, f, vac);
    };
    lc("rng_decreases_element", "an element read in a function's decreases: lowered in the Method and framed by the requires atoms in f__wf",
       [=](Kit& k)
       {
           dec_element(k, false);
       });
    lc("rng_decreases_element_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, dec_element(k, true));
       });
    // int sr(int *p, int n) requires(reads_range(p, n) && 1 <= n) ensures(reads_range(p, n) && p[0] == old(p[0])) {}   a reads_range place keeps its value
    const auto reads_keeps = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("sr", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.eq(crd(k, f, 0, N(k, "0")), k.old(crd(k, f, 0, N(k, "0"))))));
        return idle(k, f, vac);
    };
    lc("rng_reads_keeps", "p[0] == old(p[0]) under reads_range(p, n): a place borrowed by a read atom has its value back (the fact of f__wf)",
       [=](Kit& k)
       {
           reads_keeps(k, false);
       });
    lc("rng_reads_keeps_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, reads_keeps(k, true));
       });
    lc("rng_owns_changes",
       "the same with owns_range: the body writes p[0] = 5 and the ensures still claims p[0] == old(p[0]): postcondition",
       [=](Kit& k)
       {
           Function* f {k.fn("sr", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
           k.ens(f, k.land(RO(k, f, 0, 1, 0), k.eq(crd(k, f, 0, N(k, "0")), k.old(crd(k, f, 0, N(k, "0"))))));
           f->body = k.blk({bwr(k, f, 0, N(k, "0"), N(k, "5"))});
           k.done(f);
           k.fail("postcondition", f->end_loc);
       });
    // `*p` and `q[0]`: two reads, one cell: the same-address fact joins their ghosts
    const auto cell_and_element = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("ce", k.vd, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.b.atom(AtomKind::Reads, fx(k, f, 0), 0, k.at()), RR(k, f, 1, 2, 1)));
        k.req(f, k.land(k.eq(fx(k, f, 0), fx(k, f, 1)), k.ge_c(fx(k, f, 2), k.i32, "1")));
        k.req(f, k.eq(k.deref(fx(k, f, 0)), crd(k, f, 1, N(k, "0"))));
        k.ens(f, k.land(k.b.atom(AtomKind::Reads, fx(k, f, 0), 0, k.at()), RR(k, f, 1, 2, 1)));
        return idle(k, f, vac);
    };
    lc("rng_cell_and_element", "requires(reads(*p) && reads_range(q, n) && p == q && n >= 1) ensures(...) with *p == q[0]: the cell read and the element read have one value when the addresses agree",
       [=](Kit& k)
       {
           cell_and_element(k, false);
       });
    lc("rng_cell_and_element_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, cell_and_element(k, true));
       });

    // ---- item 9: the return rows of several atoms ----
    // void both(int *p, int n, int *q, int m) requires(reads_range(p, n) && reads_range(q, m)) ensures(reads_range(p, n) && reads_range(q, m)) {}
    const auto two_reads = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("both", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)));
        return idle(k, f, vac);
    };
    lc("rng_two_reads_sum", "two reads_range atoms of one field that may overlap: the marked rows carry the sum of both amounts where the other covers the slot",
       [=](Kit& k)
       {
           two_reads(k, false);
       });
    lc("rng_two_reads_sum_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, two_reads(k, true));
       });
    // void mo(int *p, int n, int *q) requires(owns_range(p, n) && owns(*q)) ensures(owns_range(p, n) && owns(*q)) {}
    const auto beside_owned = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("mo", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.b.atom(AtomKind::Owns, fx(k, f, 2), 1, k.at())));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), k.b.atom(AtomKind::Owns, fx(k, f, 2), 1, k.at())));
        return idle(k, f, vac);
    };
    lc("rng_beside_owned_cell", "owns_range(p, n) beside owns(*q): each returned, the liveness sums count both where the blocks coincide",
       [=](Kit& k)
       {
           beside_owned(k, false);
       });
    lc("rng_beside_owned_cell_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, beside_owned(k, true));
       });
    // void mr(int *p, int n, int *q) requires(reads_range(p, n) && reads(*q)) ensures(reads_range(p, n) && reads(*q)) {}
    const auto beside_read = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("mr", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())));
        return idle(k, f, vac);
    };
    lc("rng_beside_read_cell", "reads_range(p, n) beside reads(*q) with q possibly inside the range: the cell's return sum gains the range's amount where q lies in it",
       [=](Kit& k)
       {
           beside_read(k, false);
       });
    lc("rng_beside_read_cell_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, beside_read(k, true));
       });
    // void bp(int *p, int n) requires(n == 0 || owns_range(p, n)) ensures(owns_range(p, n)) {}
    // item 3: the ensures atom has no guard but its partner has one: on the path n == 0 it returns what the requires did not take
    lc("rng_partner_by_path",
       "requires(n == 0 || owns_range(p, n)) ensures(owns_range(p, n)): f__wf asserts g_ens ==> g_partner at the ensures atom (permission); on the path n == 0 the return returns what was never taken: its shape row (p may be null) and its liveness row fail too",
       [=](Kit& k)
       {
           Function* f {k.fn("bp", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(ge0(k, f, 1), k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0))));
           const Expr* ea {RO(k, f, 0, 1, 0)};
           k.ens(f, ea);
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", ea->loc);
           // the method's own rows at the closing brace (marked v, marked d, shape, flags, range, liveness): on the path n == 0 the callee holds nothing, so the shape row
           // fails and, the shape row being assumed, the liveness row fails too (the marked, flag and range rows are empty or implied)
           k.fail_only("permission", f->end_loc, 6U, 2U);
           k.fail_only("permission", f->end_loc, 6U, 5U);
       });

    // ---- item 9: a kept range (the marked return rows fail), with no call of a range function: a cell callee that keeps what it is given ----
    // void keep(int *q) requires(owns(*q)) {}   void kp(int *p, int n) requires(owns_range(p, n) && 2 <= n) ensures(owns_range(p, n)) { keep(&p[1]); }
    lc("rng_keep_owned",
       "a callee takes owns(*q) of &p[1] and keeps it: the first marked row at the return (the value cells of the range) fails alone with insufficient permission (a failed marked row ends its path)",
       [=](Kit& k)
       {
           Function* keep {k.fn("keep", k.vd, {{"q", PI(k)}})};
           k.req(keep, k.b.atom(AtomKind::Owns, fx(k, keep, 0), 0, k.at()));
           keep->body = k.blk({});
           k.done(keep);
           Function* f {k.fn("kp", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "2")));
           k.ens(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({k.es(k.call(keep, {bat(k, f, 0, N(k, "1"))}))});
           k.done(f);
           k.fail_only("permission", f->end_loc, 6U, 0U);
       });
    // void keepr(int *x) requires(reads(*x)) {}
    // void kr(int *p, int n, int *q, int m) requires(reads_range(p, n) && reads_range(q, m) && p == q && n == m && 1 <= n) ensures(reads_range(p, n) && reads_range(q, m)) { keepr(&p[0]); }
    lc("rng_keep_read",
       "a callee takes reads(*x) of &p[0] and keeps its fraction, p and q naming one range read twice: the sum-form row (both atoms' amounts at the slot) fails alone, the per-atom amount would not (S44 (e))",
       [=](Kit& k)
       {
           Function* keep {k.fn("keepr", k.vd, {{"x", PI(k)}})};
           k.req(keep, k.b.atom(AtomKind::Reads, fx(k, keep, 0), 0, k.at()));
           keep->body = k.blk({});
           k.done(keep);
           Function* f {k.fn("kr", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}})};
           k.req(f, k.land(k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)),
                           k.land(k.eq(fx(k, f, 0), fx(k, f, 2)),
                                  k.land(k.eq(fx(k, f, 1), fx(k, f, 3)), k.ge_c(fx(k, f, 1), k.i32, "1")))));
           k.ens(f, k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)));
           f->body = k.blk({k.es(k.call(keep, {bat(k, f, 0, N(k, "0"))}))});
           k.done(f);
           k.fail_only("permission", f->end_loc, 12U, 0U);
       });
    // void kc(int *p, int n, int *q) requires(reads_range(p, n) && reads(*q) && p == q && 2 <= n) ensures(reads_range(p, n) && reads(*q)) { keepr(&p[<0|1>]); }
    // q is the first element of the range, read twice: the cell's return sum counts the range's amount there (audit round 1, S6) and the range's marked rows count the cell's
    const auto keep_beside_cell = [=](Kit& k, const char* index) -> Function*
    {
        Function* keep {k.fn("keepr", k.vd, {{"x", PI(k)}})};
        k.req(keep, k.b.atom(AtomKind::Reads, fx(k, keep, 0), 0, k.at()));
        keep->body = k.blk({});
        k.done(keep);
        Function* f {k.fn("kc", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
        k.req(f, k.land(k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())),
                        k.land(k.eq(fx(k, f, 0), fx(k, f, 2)), k.ge_c(fx(k, f, 1), k.i32, "2"))));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())));
        f->body = k.blk({k.es(k.call(keep, {bat(k, f, 0, N(k, index))}))});
        k.done(f);
        return f;
    };
    lc("rng_keep_beside_cell_shared",
       "the callee keeps a fraction of the first element, which is also *q: one error at the return, the cell's value-sum row `perm(q.v) >= a_q + (q in the range ? a_p : none)` (audit round 1, S6); the failed row is assumed and ends the path",
       [=](Kit& k)
       {
           Function* f {keep_beside_cell(k, "0")};
           // the rows at the return: the cell's six (the null row, the sums of the value and flag cells, the flag, the range, the liveness sum), the range's six
           k.fail_only("permission", f->end_loc, 12U, 1U);
       });
    lc("rng_keep_beside_cell_other",
       "the callee keeps a fraction of the second element: the cell's own value cells are whole, but it kept a fraction of the block's liveness, so the cell's liveness sum (which counts the range's amount in the same block) fails alone",
       [=](Kit& k)
       {
           Function* f {keep_beside_cell(k, "1")};
           k.fail_only("permission", f->end_loc, 12U, 5U);
       });
    // the same call of a callee that gives the permission back: void borrow(int *q) requires(owns(*q)) ensures(owns(*q)) {}
    lc("rng_lend_owned", "the same call of a callee that returns what it takes (owns(*q) in its ensures too): the range comes back whole, nothing fails",
       [=](Kit& k)
       {
           Function* borrow {k.fn("borrow", k.vd, {{"q", PI(k)}})};
           k.req(borrow, k.b.atom(AtomKind::Owns, fx(k, borrow, 0), 0, k.at()));
           k.ens(borrow, k.b.atom(AtomKind::Owns, fx(k, borrow, 0), 0, k.at()));
           borrow->body = k.blk({});
           k.done(borrow);
           Function* f {k.fn("kp", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "2")));
           k.ens(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({k.es(k.call(borrow, {bat(k, f, 0, N(k, "1"))}))});
           k.done(f);
       });

    // ---- guards other than one `||`: a nested one, an arm of `?:` ----
    // void ng(int *p, int n, int m) requires(0 <= n && 0 <= m && (n == 0 || (m == 0 || owns_range(p, n)))) ensures(n == 0 || (m == 0 || owns_range(p, n))) { if (n != 0) { if (m != 0) { p[0] = 7; } } }
    const auto nested_guard = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("ng", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"m", k.i32}})};
        auto g = [=](Kit& kk, Function* ff)
        {
            return kk.lor(kk.eq_c(fx(kk, ff, 1), kk.i32, "0"), kk.lor(kk.eq_c(fx(kk, ff, 2), kk.i32, "0"), RO(kk, ff, 0, 1, 0)));
        };
        k.req(f, k.land(k.land(ge0(k, f, 1), ge0(k, f, 2)), g(k, f)));
        k.ens(f, g(k, f));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), N(k, "7"))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        const Stmt* inner {k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blk({k.iff(k.ne(k.pv(f, 2), N(k, "0")), k.blk({w}), nullptr)}), nullptr)};
        const Stmt* tail {c != nullptr ? k.iff(k.land(k.ne(k.pv(f, 1), N(k, "0")), k.ne(k.pv(f, 2), N(k, "0"))), k.blk({c}), nullptr)
                                       : nullptr};
        f->body = k.blk(tail != nullptr ? std::vector<const Stmt*> {inner, tail} : std::vector<const Stmt*> {inner});
        k.done(f);
        return c;
    };
    lc("rng_guard_nested", "a range atom under n == 0 || (m == 0 || owns_range(p, n)): both guards stand outside each quantifier as one conjoined condition",
       [=](Kit& k)
       {
           nested_guard(k, false);
       });
    lc("rng_guard_nested_vacuity", "the same with check(false) under if (n != 0 && m != 0): it fails (the guard-true path is not empty)",
       [=](Kit& k)
       {
           vacuity(k, nested_guard(k, true));
       });
    // void cg(int *p, int n) requires(0 <= n && (n != 0 ? owns_range(p, n) : true)) ensures(n != 0 ? owns_range(p, n) : true) { if (n != 0) { p[0] = 7; } }
    const auto cond_guard = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("cg", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        auto g = [=](Kit& kk, Function* ff)
        {
            return kk.cond(kk.ne(fx(kk, ff, 1), N(kk, "0")), RO(kk, ff, 0, 1, 0), kk.yes());
        };
        k.req(f, k.land(ge0(k, f, 1), g(k, f)));
        k.ens(f, g(k, f));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), N(k, "7"))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        const Stmt* inner {k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blk({w}), nullptr)};
        const Stmt* tail {c != nullptr ? k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blk({c}), nullptr) : nullptr};
        f->body = k.blk(tail != nullptr ? std::vector<const Stmt*> {inner, tail} : std::vector<const Stmt*> {inner});
        k.done(f);
        return c;
    };
    lc("rng_guard_cond", "a range atom in the arm of n != 0 ? owns_range(p, n) : true: the condition stands outside each quantifier",
       [=](Kit& k)
       {
           cond_guard(k, false);
       });
    lc("rng_guard_cond_vacuity", "the same with check(false) under if (n != 0): it fails (the guard-true path is not empty)",
       [=](Kit& k)
       {
           vacuity(k, cond_guard(k, true));
       });

    // ---- loops that the staging pin S2 does not name: no memory touched in the loop ----
    // void lp(int *p, int n) requires(owns_range(p, n) && 0 <= n) ensures(owns_range(p, n)) { int i = 0; while (i < n) decreases(n - i) { i = i + 1; } }
    lc("rng_loop_no_memory", "a loop that touches no memory in a function with a range atom: no place is carried, the range is in the frame",
       [=](Kit& k)
       {
           Function* f {k.fn("lp", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), ge0(k, f, 1)));
           k.ens(f, RO(k, f, 0, 1, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blk({k.asg(i, k.add(k.v(i), N(k, "1")))})};
           f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), k.pv(f, 1)), {}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
           k.done(f);
       });
    // the same with a `return` in the body: void lr(int *p, int n) ... { int i = 0; while (i < n) decreases(n - i) { if (i == 3) { return; } i = i + 1; } }
    rh("range_hold_loop_return", "before Part D of record 0060",
       [=](Kit& k)
       {
           // the loop touches no memory, but it leaves the function while the function holds a place, so it carries every place (record 0038 item 3): the staging gate fires, and S2 does not name it
           Function* f {k.fn("lr", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), ge0(k, f, 1)));
           k.ens(f, RO(k, f, 0, 1, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blk({k.iff(k.eq_c(k.v(i), k.i32, "3"), k.blk({k.ret(nullptr)}), nullptr),
                                    k.asg(i, k.add(k.v(i), N(k, "1")))})};
           f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), k.pv(f, 1)), {}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
           k.done(f);
       });

    // ---- a range beside an output atom ----
    // int *oa(int *p, int n) requires(reads_range(p, n) && 1 <= n) ensures(reads_range(p, n) && reads(*result)) { return &p[0]; }
    lc("rng_output_atom",
       "a reads_range borrowed and returned whole beside an ensures atom on the result: the function holds only the range's amount, nothing is left to give the output atom: the explicit `without holding it` row (the first row at the return, an ending one) fails, not Silicon's own check of the postcondition",
       [=](Kit& k)
       {
           Function* f {k.fn("oa", PI(k), {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0, 1, 0), k.ge_c(fx(k, f, 1), k.i32, "1")));
           k.ens(f, k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, k.R(f), 1, k.at())));
           const Stmt* r {k.ret(bat(k, f, 0, N(k, "0")))};
           f->body = k.blk({r});
           k.done(f);
           // the rows at the return: the output atom's ending row, its cell's rows, the range's six
           k.fail_only("permission", r->loc, 13U, 0U);
       });

    // ---- count obligations (0010 item 11) ----
    // void ob(int *p, int n) requires([n < 100 &&] owns_range(p, n + 1)) ensures(owns_range(p, n + 1)) {}
    const auto count_overflow = [=](Kit& k, bool bounded) -> std::pair<const Expr*, const Expr*>
    {
        Function* f {k.fn("ob", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        const Expr* c1 {k.add(fx(k, f, 1), N(k, "1"))};
        const Expr* c2 {k.add(fx(k, f, 1), N(k, "1"))};
        const Expr* a1 {k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), c1, 0, k.at())};
        const Expr* a2 {k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), c2, 0, k.at())};
        k.req(f, bounded ? k.land(k.lt(fx(k, f, 1), N(k, "100")), a1) : a1);
        k.ens(f, a2);
        f->body = k.blk({});
        k.done(f);
        return {c1, c2};
    };
    lc("rng_count_bounded", "owns_range(p, n + 1) with n < 100: the count cannot overflow int",
       [=](Kit& k)
       {
           count_overflow(k, true);
       });
    lc("rng_count_overflow",
       "the same without the bound: n + 1 may overflow in f__wf: the requires' count fails, and the ensures' own (a node of its own, asserted) holds because the failed row is assumed (N1)",
       [=](Kit& k)
       {
           const auto [c1, c2] {count_overflow(k, false)};
           k.fail("overflow", c1->loc);
           k.rows("overflow", c2->loc, 1U);
       });

    // ---- what is still refused: the checker's rejections, one per rule of item 3, and the staging backstops ----
    auto PS = [](Kit& k, const char* tag)
    {
        return k.ptr(k.st(k.strukt(tag, {{"x", k.i32}})));
    };
    rc("range_rej_in_check", "outside the `requires` and `ensures` of a function",
       [=](Kit& k)
       {
           // check(owns_range(p, n)) in a body
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->body = k.blk({k.chk(RO(k, f, 0, 1, 0))});
           k.done(f);
       });
    rc("range_rej_in_invariant", "outside the `requires` and `ensures` of a function",
       [=](Kit& k)
       {
           // while (1) invariant(owns_range(q, n)) {}  with a pointer variable q
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           Var* q {k.local(f, "q", PI(k))};
           const Expr* atom {k.b.range_atom(AtomKind::OwnsRange, k.pn(q), k.pv(f, 1), -1, k.at())};
           f->body = k.blk({k.decl(q, k.pv(f, 0)), k.whl(k.yes(), {k.cl(atom)}, {}, k.blk({}))});
           k.done(f);
       });
    rc("range_rej_partnerless", "without a `requires` range atom",
       [=](Kit& k)
       {
           // ensures(owns_range(p, n)) and nothing in the requires
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.ens(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_count_mismatch", "whose partner in the `requires` is of another kind or on another parameter",
       [=](Kit& k)
       {
           // requires(owns_range(p, n)) ensures(owns_range(p, n + 1))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), k.add(fx(k, f, 1), N(k, "1")), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_kind_mismatch", "whose partner in the `requires` is of another kind or on another parameter",
       [=](Kit& k)
       {
           // requires(owns_range(p, n)) ensures(reads_range(p, n))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, RR(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_range_and_cell", "a range permission atom and a cell atom on the same parameter",
       [=](Kit& k)
       {
           // requires(owns_range(p, n) && owns(*p))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), k.b.atom(AtomKind::Owns, fx(k, f, 0), 1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_both_kinds", "two range permission atoms on one parameter in one clause list",
       [=](Kit& k)
       {
           // requires(owns_range(p, n) && reads_range(p, n))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), RR(k, f, 0, 1, 1)));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_same_kind_twice", "two range permission atoms on one parameter in one clause list of one kind",
       [=](Kit& k)
       {
           // requires(owns_range(p, n) && owns_range(p, m))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"m", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), RO(k, f, 0, 2, 1)));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_count_reads_memory", "reads memory, names the result or is under `old`",
       [=](Kit& k)
       {
           // requires(owns_range(p, *q)) with an int *q
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"q", PI(k)}})};
           k.req(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), k.deref(fx(k, f, 1)), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_count_result", "reads memory, names the result or is under `old`",
       [=](Kit& k)
       {
           // requires(owns_range(p, n)) ensures(owns_range(p, result))
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), k.R(f), 0, k.at()));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("range_rej_count_bool", "not an integer",
       [=](Kit& k)
       {
           // requires(owns_range(p, b)) with a bool b
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"b", k.bl}})};
           k.req(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_base_loaded", "a range permission atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(owns_range(*pp, n)) with an int **pp
           Function* f {k.fn("f", k.vd, {{"pp", k.ptr(PI(k))}, {"n", k.i32}})};
           k.req(f, k.b.range_atom(AtomKind::OwnsRange, k.deref(fx(k, f, 0)), fx(k, f, 1), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_struct_pointee", "a range permission atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(owns_range(sp, n)) with a struct S *sp
           Function* f {k.fn("f", k.vd, {{"sp", PS(k, "S")}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_guard_reads_memory", "the guard of a range permission atom reads memory",
       [=](Kit& k)
       {
           // requires(*q == 0 || owns_range(p, n))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
           k.req(f, k.land(k.b.atom(AtomKind::Reads, fx(k, f, 2), 0, k.at()),
                           k.lor(k.eq_c(k.deref(fx(k, f, 2)), k.i32, "0"), RO(k, f, 0, 1, 1))));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_marked", "beside a range permission atom",
       [=](Kit& k)
       {
           // CERASTES_REF_TO_UNINIT on a parameter whose atom is a range
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_elem_index_reads_memory", "whose index reads memory",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && p[*q] > 0)
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
           k.req(f, k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())));
           k.req(f, k.gt(crd(k, f, 0, k.deref(fx(k, f, 2))), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_elem_of_result", "an element read in a contract of anything but a pointer parameter",
       [=](Kit& k)
       {
           // ensures(result[0] > 0) with an int *result
           Function* f {k.fn("f", PI(k), {{"p", PI(k)}})};
           k.ens(f, k.gt(k.deref(k.b.elem_addr(k.R(f), N(k, "0"), k.at())), N(k, "0")));
           f->body = k.blk({k.ret(k.pv(f, 0))});
           k.done(f);
       });
    rc("range_rej_elem_address_value", "used as a value",
       [=](Kit& k)
       {
           // requires(&p[1] == p): the address of an element as a value
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           k.req(f, k.eq(k.b.elem_addr(fx(k, f, 0), N(k, "1"), k.at()), fx(k, f, 0)));
           f->body = k.blk({});
           k.done(f);
       });
    // the staging backstops: lowering refuses what Parts C and D lift
    rh("range_hold_call", "before Part C of record 0060",
       [=](Kit& k)
       {
           // void h(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) {}   void g(int *q, int m) requires(owns_range(q, m)) ensures(owns_range(q, m)) { h(q, m); }
           Function* h {k.fn("h", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(h, RO(k, h, 0, 1, 0));
           k.ens(h, RO(k, h, 0, 1, 0));
           h->body = k.blk({});
           k.done(h);
           Function* g {k.fn("g", k.vd, {{"q", PI(k)}, {"m", k.i32}})};
           k.req(g, RO(k, g, 0, 1, 0));
           k.ens(g, RO(k, g, 0, 1, 0));
           g->body = k.blk({k.es(k.call(h, {k.pv(g, 0), k.pv(g, 1)}))});
           k.done(g);
       });
    rh("range_hold_loop", "before Part D of record 0060",
       [=](Kit& k)
       {
           // void f(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) { int i = 0; while (i < n) { p[i] = 0; i = i + 1; } }
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, RO(k, f, 0, 1, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blk({bwr(k, f, 0, k.v(i), N(k, "0")), k.asg(i, k.add(k.v(i), N(k, "1")))})};
           f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), k.pv(f, 1)), {}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
           k.done(f);
       });
}

} // namespace
