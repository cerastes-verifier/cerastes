// (included by selftest.cpp) Ranges in contracts, M4.5e1b Part B (record 0060 items 3, 4, 5, 7 and 9): `owns_range(p, n)` and `reads_range(p, n)` in a function's `requires`
// and `ensures` (the pointer a parameter, the count an integer expression that reads no memory), the element reads `p[e]` of a pointer parameter in a contract, the
// callee's side of them.  A range atom lowers to one quantifier per fact over the absolute slot (`forall k :: { loc(block_of(p), k) } offset_of(p) <= k < offset_of(p) + n
// ==> ...`), a guard outside each, one conjunct per `requires` and `ensures` line; its `f__wf` is the pure line; a read `p[e]` is framed by a range atom's interval (or a cell
// atom's cell) and has one ghost value per base, index and state; at a return the marked rows (value then flag, the sum form where two read atoms may overlap) stand inside
// `if (g) { }`, then the shape row, the flags, the type's range and the liveness sum.
// The loops of such a function are `define_range_loops` (Part D) and the callers of range functions `define_range_calls` (Part C).
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
    lc("rng_get_set",
       "p[i] = v; return p[i] under owns_range(p, n) and 0 <= i < n: the bound, permission and flag rows hold, the range comes back",
       [=](Kit& k)
       {
           get_set(k, false, false);
       });
    lc("rng_get_set_wrong", "the same claiming result == v + 1: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", get_set(k, true, false)->loc);
       });
    lc("rng_get_set_vacuity",
       "the same with check(false) before the return: it fails (the path through the contract is not empty)",
       [=](Kit& k)
       {
           vacuity(k, get_set(k, false, true));
       });
    // void st(int *p, int n, int i) requires(owns_range(p, n) && 0 <= i && i <= [n - 1|n]) ensures(owns_range(p, n)) { p[i] = 0; }
    const auto store_at = [=](Kit& k, bool past) -> const Stmt*
    {
        Function* f {k.fn("st", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"i", k.i32}})};
        k.req(f,
              k.land(RO(k, f, 0, 1, 0), k.land(ge0(k, f, 2), past ? k.le(fx(k, f, 2), fx(k, f, 1)) : lt(k, f, 2, 1))));
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
    lc("rng_reads_read",
       "return p[i] under reads_range(p, n), the ensures reads p[i] too: the read is framed by the range, the body's read equals it",
       [=](Kit& k)
       {
           read_at(k, false, false);
       });
    lc("rng_reads_read_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity(k, read_at(k, false, true));
       });
    lc("rng_reads_write",
       "the same body writes p[i] = 0: reads_range holds a fraction only, the write permission row fails (permission at the store)",
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
    lc("rng_guard_n",
       "under n == 0 || owns_range(p, n) the write p[0] = 7 inside if (n != 0): the guard stands outside every quantifier, the marked rows inside if (g)",
       [=](Kit& k)
       {
           guarded(k, false, false);
       });
    lc("rng_guard_n_vacuity",
       "the same with check(false) under if (n != 0) after the write: it fails (the guard-true path is not empty)",
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
        f->body =
            k.blk(c != nullptr ? std::vector<const Stmt*> {k.iff(k.ne(k.pv(f, 0), k.nul(PI(k))), k.blk({c}), nullptr)}
                               : std::vector<const Stmt*> {});
        k.done(f);
        return c;
    };
    lc("rng_guard_p",
       "under p == NULL || owns_range(p, n) and 0 <= n: nothing to do when p is null; the body does nothing, the return rows hold under the guard",
       [=](Kit& k)
       {
           guard_p(k, false);
       });
    lc("rng_guard_p_vacuity",
       "the same with check(false) under if (p != NULL): it fails (the guard-true path is not empty)",
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
    lc("rng_enum_elements",
       "owns_range over an enumeration's elements: the type's range is the enumeration's [0, 2], stored value 1 is in it",
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
    lc("rng_ptr_elements",
       "owns_range over pointer elements: WF of the stored pointer in place of the range, a null element is well-formed",
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
    lc("rng_read_by_cell",
       "requires(owns(*p) && p[0] > 0): the element read is framed by the cell p owns (the address of p[0] is p)",
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
        k.req(f, k.land(RR(k, f, 0, 1, 0),
                        k.land(k.land(ge0(k, f, 2), past ? k.le(fx(k, f, 2), fx(k, f, 1)) : lt(k, f, 2, 1)),
                               k.ge_c(r, k.i32, "0"))));
        k.ens(f, RR(k, f, 0, 1, 0));
        const Stmt* c {idle(k, f, vc != nullptr)};
        if (vc != nullptr)
        {
            *vc = c;
        }
        return r;
    };
    lc("rng_read_in_range",
       "requires(reads_range(p, n) && 0 <= i < n && p[i] >= 0): the read is inside the interval, its framing assertion holds",
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
    lc("rng_old_element",
       "p[i] = 0 keeps p[j] for j != i: ensures p[j] == old(p[j]), two reads of one index with their ghosts (pre and post) and the equal-address facts",
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
    lc("rng_decreases_element",
       "an element read in a function's decreases: lowered in the Method and framed by the requires atoms in f__wf",
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
    lc("rng_reads_keeps",
       "p[0] == old(p[0]) under reads_range(p, n): a place borrowed by a read atom has its value back (the fact of f__wf)",
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
    lc("rng_cell_and_element",
       "requires(reads(*p) && reads_range(q, n) && p == q && n >= 1) ensures(...) with *p == q[0]: the cell read and the element read have one value when the addresses agree",
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
    lc("rng_two_reads_sum",
       "two reads_range atoms of one field that may overlap: the marked rows carry the sum of both amounts where the other covers the slot",
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
    lc("rng_beside_owned_cell",
       "owns_range(p, n) beside owns(*q): each returned, the liveness sums count both where the blocks coincide",
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
    lc("rng_beside_read_cell",
       "reads_range(p, n) beside reads(*q) with q possibly inside the range: the cell's return sum gains the range's amount where q lies in it",
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
    lc("rng_lend_owned",
       "the same call of a callee that returns what it takes (owns(*q) in its ensures too): the range comes back whole, nothing fails",
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
            return kk.lor(kk.eq_c(fx(kk, ff, 1), kk.i32, "0"),
                          kk.lor(kk.eq_c(fx(kk, ff, 2), kk.i32, "0"), RO(kk, ff, 0, 1, 0)));
        };
        k.req(f, k.land(k.land(ge0(k, f, 1), ge0(k, f, 2)), g(k, f)));
        k.ens(f, g(k, f));
        const Stmt* w {bwr(k, f, 0, N(k, "0"), N(k, "7"))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        const Stmt* inner {k.iff(k.ne(k.pv(f, 1), N(k, "0")),
                                 k.blk({k.iff(k.ne(k.pv(f, 2), N(k, "0")), k.blk({w}), nullptr)}), nullptr)};
        const Stmt* tail {
            c != nullptr ? k.iff(k.land(k.ne(k.pv(f, 1), N(k, "0")), k.ne(k.pv(f, 2), N(k, "0"))), k.blk({c}), nullptr)
                         : nullptr};
        f->body = k.blk(tail != nullptr ? std::vector<const Stmt*> {inner, tail} : std::vector<const Stmt*> {inner});
        k.done(f);
        return c;
    };
    lc("rng_guard_nested",
       "a range atom under n == 0 || (m == 0 || owns_range(p, n)): both guards stand outside each quantifier as one conjoined condition",
       [=](Kit& k)
       {
           nested_guard(k, false);
       });
    lc("rng_guard_nested_vacuity",
       "the same with check(false) under if (n != 0 && m != 0): it fails (the guard-true path is not empty)",
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
    lc("rng_guard_cond",
       "a range atom in the arm of n != 0 ? owns_range(p, n) : true: the condition stands outside each quantifier",
       [=](Kit& k)
       {
           cond_guard(k, false);
       });
    lc("rng_guard_cond_vacuity",
       "the same with check(false) under if (n != 0): it fails (the guard-true path is not empty)",
       [=](Kit& k)
       {
           vacuity(k, cond_guard(k, true));
       });

    // ---- loops that the staging pin S2 does not name: no memory touched in the loop ----
    // void lp(int *p, int n) requires(owns_range(p, n) && 0 <= n) ensures(owns_range(p, n)) { int i = 0; while (i < n) decreases(n - i) { i = i + 1; } }
    lc("rng_loop_no_memory",
       "a loop that touches no memory in a function with a range atom: no place is carried, the range is in the frame",
       [=](Kit& k)
       {
           Function* f {k.fn("lp", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0, 1, 0), ge0(k, f, 1)));
           k.ens(f, RO(k, f, 0, 1, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blk({k.asg(i, k.add(k.v(i), N(k, "1")))})};
           f->body = k.blk(
               {k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), k.pv(f, 1)), {}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
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
    rc("range_rej_marked_reads_range", "beside `reads_range(p, n)`",
       [=](Kit& k)
       {
           // CERASTES_REF_TO_UNINIT on a parameter whose atom is a reads_range: refused like a marker beside reads(*p)
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, RR(k, f, 0, 1, 0));
           k.ens(f, RR(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_marked_must_init_no_ensures", "is `must_init` without `owns(*p)` in its `ensures`",
       [=](Kit& k)
       {
           // CERASTES_MUST_INIT on owns_range(p, n) whose ensures does not return it
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::MustInit;
           k.req(f, RO(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("range_rej_marked_element_read", "a contract reads `*p`",
       [=](Kit& k)
       {
           // CERASTES_REF_TO_UNINIT on owns_range(p, n) with requires(p[0] > 0): the element of a marked parameter has no value to read
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(k.land(RO(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                           k.gt(crd(k, f, 0, N(k, "0")), N(k, "0"))));
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
}

// ---- Part D: loops over the ranges of parameters (record 0060 item 11) ----
// A loop of a function with a `requires` range atom of a pointer parameter the function never assigns carries a range place: its marked entry rows ("this loop begins without the
// permission to every element of `p[0..n)`", at the loop), the permissions (at `write`, or at the atom's own amount), the flags kept, the type's range and (for a place no write
// reaches, or a read-only one) the values kept, all under the atom's guard, then the marked back-edge rows (at the end of the body); the liveness is the parameter's `live`
// place's.  A cell place of the range's field gains "not inside the range" on every conjunct and its back-edge row; where two read places may share a slot the marked rows take the
// sum form.  `p[e]` of any pointer in an invariant or a measure is framed by a place (a cell's or a range's interval test), else `permission` at the clause.
void define_range_loops()
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
    auto RO = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    // `p[e]` as a read in a contract, and in a body (of the parameter's local copy)
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
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
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
    // 0 <= i && i <= n, the loop's usual invariant over parameter `n`, and its measure n - i
    auto bounds = [=](Kit& k, Function* f, Var* i, int n)
    {
        return k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, n)));
    };
    // a statement list ending in the vacuity twin when `vac` (check(false)), which must fail: the path it stands on is not empty
    auto vac_fail = [](Kit& k, const Stmt* c)
    {
        k.fail("check", c->loc);
    };

    // ---- the fill loop over an owned range ----
    // void fill(int *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while ([i < n|i <= n]) inv 0 <= i <= [n|100] decreases n - i { p[i] = i; [check(false);] i++; } [check(false);] }
    // `mode`: 0 the fill, 1 i <= n, 2 the check(false) after the loop, 3 the check(false) at the end of the body
    const auto fill = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("fill", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0, 1, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {bwr(k, f, 0, k.v(i), k.v(i))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c3 != nullptr)
        {
            bs.push_back(c3);
        }
        bs.push_back(inc(k, i));
        const Expr* cond {mode == 1 ? k.le(k.v(i), k.pv(f, 1)) : k.lt(k.v(i), k.pv(f, 1))};
        const Expr* inv {mode == 1 ? k.range(k.v(i), k.i32, "0", "100") : bounds(k, f, i, 1)};
        const Stmt* loop {k.whl(cond, {k.cl(inv)}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 2 ? c2 : mode == 3 ? c3 : w;
    };
    lc("rngd_fill",
       "while (i < n) p[i] = i over owns_range(p, n): the range place carries the permissions, the flags and the type's range through the loop; its marked entry and back-edge rows hold",
       [=](Kit& k)
       {
           fill(k, 0);
       });
    lc("rngd_fill_high",
       "the loop runs to i <= n: p[n] fails its bound row and its write-permission row at the store (two real cases at one line, N1)",
       [=](Kit& k)
       {
           const Stmt* w {fill(k, 1)};
           k.fail("out-of-bounds", w->loc);
           k.fail("permission", w->loc);
       });
    lc("rngd_fill_vacuity",
       "the fill with check(false) after the loop: it fails (the path through the loop is not empty)",
       [=](Kit& k)
       {
           vac_fail(k, fill(k, 2));
       });
    lc("rngd_fill_body_vacuity",
       "the fill with check(false) at the end of the body: it fails (an iteration is possible; the body is not vacuous)",
       [=](Kit& k)
       {
           vac_fail(k, fill(k, 3));
       });

    // ---- the values: kept where no write reaches the field or the place is read-only, lost for a written one ----
    // int vals(int *p, int n) requires(owns_range(p, n) && 0 < n && p[0] == 7) ensures(owns_range(p, n) && ...) { int i = 0; int s = 0;
    //   while (i < n) inv 0 <= i <= n && [s == s] { [p[i] = 0;|s = s + p[i];] i++; } check(p[0] == [7|8]); return s; }
    // `write`: the loop writes p[i]; the values are lost then (the check fails), and kept otherwise
    const auto values = [=](Kit& k, bool write, const char* want, bool reads, bool vac) -> const Stmt*
    {
        Function* f {k.fn("vals", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        const Expr* atom {reads ? RR(k, f, 0, 1, 0) : RO(k, f, 0, 1, 0)};
        k.req(f, k.land(k.land(atom, k.gt(fx(k, f, 1), N(k, "0"))), k.eq_c(crd(k, f, 0, N(k, "0")), k.i32, "7")));
        k.ens(f, k.land(reads ? RR(k, f, 0, 1, 0) : RO(k, f, 0, 1, 0), k.ge_c(k.R(f), k.i32, "0")));
        Var* i {k.local(f, "i", k.i32)};
        Var* s {k.local(f, "s", k.i32)};
        std::vector<const Stmt*> bs;
        if (write)
        {
            bs.push_back(bwr(k, f, 0, k.v(i), N(k, "0")));
        }
        else
        {
            bs.push_back(
                k.iff(k.eq_c(brd(k, f, 0, k.v(i)), k.i32, "7"), k.blkc({k.asg(s, k.add(k.v(s), N(k, "1")))}), nullptr));
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)),
                  {k.cl(k.land(bounds(k, f, i, 1), k.land(k.ge_c(k.v(s), k.i32, "0"), k.le(k.v(s), k.v(i)))))},
                  {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c {k.chk(vac ? k.no() : k.eq_c(brd(k, f, 0, N(k, "0")), k.i32, want))};
        f->body = k.blk({k.decl(i, N(k, "0")), k.decl(s, N(k, "0")), loop, c, k.ret(k.v(s))});
        k.done(f);
        return c;
    };
    lc("rngd_values_kept_reads",
       "a read-only loop over reads_range(p, n) keeps the values (the read-only place's values-kept conjunct): p[0] == 7 still holds after it",
       [=](Kit& k)
       {
           values(k, false, "7", true, false);
       });
    lc("rngd_values_kept_reads_wrong", "the same checking p[0] == 8: fails at the check",
       [=](Kit& k)
       {
           k.fail("check", values(k, false, "8", true, false)->loc);
       });
    lc("rngd_values_kept_reads_vacuity", "the same with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, values(k, false, "7", true, true));
       });
    lc("rngd_values_kept_owns",
       "a loop that only reads an owned range keeps the values too (no write reaches the field)",
       [=](Kit& k)
       {
           values(k, false, "7", false, false);
       });
    lc("rngd_values_lost_owns",
       "a loop that writes p[i] over an owned range loses the values: p[0] == 7 is not known after it (check)",
       [=](Kit& k)
       {
           k.fail("check", values(k, true, "7", false, false)->loc);
       });

    // ---- the search: a read place, a return in the body ----
    // int search(int *p, int n, int x) requires(reads_range(p, n)) ensures(reads_range(p, n) && (result == -1 || (0 <= result && result < n && p[result] [==|!=] x)))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { if (p[i] == x) { [check(false);] return i; } i++; } [check(false);] return -1; }
    // `mode`: 0 the search, 1 the ensures says p[r] != x, 2 check(false) in the branch before the return, 3 check(false) after the loop
    const auto search = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("search", k.i32, {{"p", PI(k)}, {"n", k.i32}, {"x", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        const Expr* hit {crd(k, f, 0, k.R(f))};
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.lor(k.eq_c(k.R(f), k.i32, "-1"),
                                                 k.land(k.land(k.ge_c(k.R(f), k.i32, "0"), k.lt(k.R(f), fx(k, f, 1))),
                                                        mode == 1 ? k.ne(hit, fx(k, f, 2)) : k.eq(hit, fx(k, f, 2))))));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        const Stmt* r {k.ret(k.v(i))};
        std::vector<const Stmt*> hits;
        if (c2 != nullptr)
        {
            hits.push_back(c2);
        }
        hits.push_back(r);
        const Stmt* body {k.blkc({k.iff(k.eq(brd(k, f, 0, k.v(i)), k.pv(f, 2)), k.blkc(hits), nullptr), inc(k, i)})};
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        const Stmt* last {k.ret(N(k, "-1"))};
        std::vector<const Stmt*> bs {k.decl(i, N(k, "0")), loop};
        if (c3 != nullptr)
        {
            bs.push_back(c3);
        }
        bs.push_back(last);
        f->body = k.blk(std::move(bs));
        k.done(f);
        return mode == 1 ? r : mode == 2 ? c2 : mode == 3 ? c3 : nullptr;
    };
    lc("rngd_search",
       "a loop over reads_range(p, n) with a return in its body: the return rows run inside the body, the ensures reads p[result] under result != -1 (framed by the range), the values are kept",
       [=](Kit& k)
       {
           search(k, 0);
       });
    lc("rngd_search_wrong",
       "the ensures claims p[result] != x where the loop returned the index of x: the postcondition fails at that return only",
       [=](Kit& k)
       {
           k.fail("postcondition", search(k, 1)->loc);
       });
    lc("rngd_search_vacuity_branch",
       "the search with check(false) before the return in the loop: it fails (the branch is reachable)",
       [=](Kit& k)
       {
           vac_fail(k, search(k, 2));
       });
    lc("rngd_search_vacuity_after", "the search with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, search(k, 3));
       });

    // ---- two range parameters: a copy, and a read place written ----
    // void copy(int *dst, int *src, int n) requires(owns_range(dst, n) && reads_range(src, n)) ensures(the same)
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { [dst[i] = src[i];|src[i] = 0;] [check(false);] i++; } [check(false);] }
    const auto copy = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("copy", k.vd, {{"dst", PI(k)}, {"src", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
        k.ens(f, k.land(RO(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {mode == 1 ? bwr(k, f, 1, k.v(i), N(k, "0")) : bwr(k, f, 0, k.v(i), brd(k, f, 1, k.v(i)))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c3 != nullptr)
        {
            bs.push_back(c3);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 2)), {k.cl(bounds(k, f, i, 2))}, {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 1 ? w : mode == 2 ? c2 : mode == 3 ? c3 : nullptr;
    };
    lc("rngd_copy",
       "dst[i] = src[i] over owns_range(dst, n) and reads_range(src, n): two range places, the entry liveness split, both given back",
       [=](Kit& k)
       {
           copy(k, 0);
       });
    lc("rngd_copy_write_src",
       "the loop writes the read-only range (src[i] = 0): the write's permission row fails at the store (permission)",
       [=](Kit& k)
       {
           k.fail("permission", copy(k, 1)->loc);
       });
    lc("rngd_copy_vacuity", "the copy with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, copy(k, 2));
       });
    lc("rngd_copy_body_vacuity", "the copy with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, copy(k, 3));
       });

    // ---- a guarded range: the conjuncts and the marked rows under the guard ----
    // void fill_g(int *p, int n) requires(n == 0 || owns_range(p, n)) ensures(n == 0 || owns_range(p, n) [&& (n == 0 || p[0] == 0)])
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i] = i; [if (n != 0) check(false);] i++; } [if (n != 0) check(false);] }
    // `mode`: 0 the fill, 1 a second ensures claims p[0] == 0 under the guard (the loop wrote p[i] = i and keeps no value), 2 check(false) under the guard in the body,
    // 3 check(false) under the guard after the loop
    const auto guarded = [=](Kit& k, int mode, SrcLoc* end = nullptr) -> const Stmt*
    {
        Function* f {k.fn("fill_g", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)));
        const Expr* atom {k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0))};
        k.ens(f, mode == 1
                     ? k.land(atom, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), k.eq_c(crd(k, f, 0, N(k, "0")), k.i32, "0")))
                     : atom);
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {bwr(k, f, 0, k.v(i), k.v(i))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c2 != nullptr)
        {
            bs.push_back(k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blkc({c2}), nullptr));
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), loop};
        if (c3 != nullptr)
        {
            body.push_back(k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blkc({c3}), nullptr));
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        if (end != nullptr)
        {
            *end = f->end_loc;
        }
        return mode == 2 ? c2 : mode == 3 ? c3 : nullptr;
    };
    lc("rngd_guarded",
       "a loop over n == 0 || owns_range(p, n): the conjuncts and the marked rows are under the guard (if (g) { }), the liveness is the parameter's place",
       [=](Kit& k)
       {
           guarded(k, 0);
       });
    lc("rngd_guarded_post",
       "a second ensures claims p[0] == 0 under the guard: the loop wrote p[i] = i and keeps no value, so the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           SrcLoc end;
           guarded(k, 1, &end);
           k.fail("postcondition", end);
       });
    lc("rngd_guarded_body_vacuity",
       "check(false) under n != 0 at the end of the body: it fails (the guard-true iteration is possible)",
       [=](Kit& k)
       {
           vac_fail(k, guarded(k, 2));
       });
    lc("rngd_guarded_after_vacuity", "check(false) under n != 0 after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, guarded(k, 3));
       });

    // ---- a guarded range beside an address-taken local and a local array of the same field (0038's audit round 2 and e1a's shape) ----
    // int beside(int *p, int n) requires(n == 0 || owns_range(p, n)) ensures((n == 0 || owns_range(p, n)) && result == [0|1])
    //   { int x = 1; int *px = &x; int a[3] = {1, 2, 3}; int i = 0;
    //     while (i < n) inv 0 <= i <= n decreases n - i { p[i] = i; *px = i; a[[0|i]] = i; i++; } return a[1] - a[1]; }
    // `mode`: 0 the shape, 1 a[i] with i up to n (the array's bound row), 2 the ensures claims result == 1, 3 check(false) at the end of the body
    const auto beside = [=](Kit& k, int mode, const Function** fout = nullptr) -> const Stmt*
    {
        Function* f {k.fn("beside", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)));
        k.ens(f, k.land(k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)),
                        k.eq_c(k.R(f), k.i32, mode == 2 ? "1" : "0")));
        Var* x {k.cell(f, "x", k.i32)};
        Var* px {k.local(f, "px", PI(k))};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* wa {awr(k, a, mode == 1 ? k.v(i) : N(k, "0"), k.v(i))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {bwr(k, f, 0, k.v(i), k.v(i)), k.store(k.pn(px), k.v(i)), wa};
        if (c3 != nullptr)
        {
            bs.push_back(k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blkc({c3}), nullptr));
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* last {k.ret(k.sub(ard(k, a, N(k, "1")), ard(k, a, N(k, "1"))))};
        f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(px, k.addr(x)), k.decl(a, nullptr),
                          awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                          awr(k, a, N(k, "2"), N(k, "3")), k.decl(i, N(k, "0")), loop, last});
        k.done(f);
        if (fout != nullptr)
        {
            *fout = f;
        }
        return mode == 1 ? wa : mode == 2 ? last : c3;
    };
    lc("rngd_beside",
       "a guarded range, an address-taken local and a local array of one field all written in one loop: three places, the range's rows inside if (g), no exit 3 for either guard value",
       [=](Kit& k)
       {
           beside(k, 0);
       });
    lc("rngd_beside_index",
       "the body stores a[i] with i up to n into the three-element array: the array's bound row fails at the store, one error",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", beside(k, 1)->loc);
       });
    lc("rngd_beside_post",
       "the ensures claims result == 1 where the function returns 0: the postcondition fails at the return, one error",
       [=](Kit& k)
       {
           k.fail("postcondition", beside(k, 2)->loc);
       });
    lc("rngd_beside_body_vacuity",
       "check(false) under n != 0 at the end of the body: it fails (the guard-true iteration is reachable)",
       [=](Kit& k)
       {
           vac_fail(k, beside(k, 3));
       });

    // ---- p[e] in an invariant and in a measure ----
    // void countdown(int *p, int n) requires(owns_range(p, n) && n > 0 && 0 <= p[0] && p[0] <= 100) ensures(owns_range(p, n) && p[0] == 0)
    //   { while (p[0] > 0) inv 0 <= p[0] && p[0] <= 100 decreases p[0] { p[0] -= 1; [check(false);] } [check(false);] }
    // `mode`: 0 the countdown, 1 check(false) at the end of the body, 2 check(false) after the loop
    const auto countdown = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("countdown", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RO(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                        k.land(k.ge_c(crd(k, f, 0, N(k, "0")), k.i32, "0"),
                               k.le_c(crd(k, f, 0, N(k, "0")), k.i32, "100"))));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), k.eq_c(crd(k, f, 0, N(k, "0")), k.i32, "0")));
        const Stmt* w {k.cstore(bat(k, f, 0, N(k, "0")), BinaryOp::Sub, N(k, "1"))};
        const Stmt* c1 {mode == 1 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c1 != nullptr)
        {
            bs.push_back(c1);
        }
        const Expr* cur {brd(k, f, 0, N(k, "0"))};
        const Stmt* loop {k.whl(
            k.gt(cur, N(k, "0")),
            {k.cl(k.land(k.ge_c(brd(k, f, 0, N(k, "0")), k.i32, "0"), k.le_c(brd(k, f, 0, N(k, "0")), k.i32, "100")))},
            {k.cl(brd(k, f, 0, N(k, "0")))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 1 ? c1 : c2;
    };
    lc("rngd_element_clauses",
       "p[0] in a loop's invariant and in its measure over owns_range(p, n): the reads are framed by the range place's interval test",
       [=](Kit& k)
       {
           countdown(k, 0);
       });
    lc("rngd_element_clauses_body_vacuity", "the countdown with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, countdown(k, 1));
       });
    lc("rngd_element_clauses_vacuity", "the countdown with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, countdown(k, 2));
       });

    // int first_negative(int *p, int n) requires(reads_range(p, n)) ensures(reads_range(p, n) && (result == -1 || (0 <= result && result < n && p[result] < 0)))
    //   { int i = 0; while (i < n) inv 0 <= i <= n && (i == 0 || p[i - 1] >= 0) decreases n - i { if (p[i] < 0) { return i; } i++; } return -1; }
    // `unframed`: the invariant is `i < n || p[i] >= 0`, which reads p[i] where i is n (and is inductive: the unframed read is the dummy value 0, the only failure the framing)
    const auto first_negative = [=](Kit& k, bool unframed, bool vac, SrcLoc* vac_at = nullptr) -> const Expr*
    {
        Function* f {k.fn("first_negative", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), k.lor(k.eq_c(k.R(f), k.i32, "-1"),
                                                 k.land(k.land(k.ge_c(k.R(f), k.i32, "0"), k.lt(k.R(f), fx(k, f, 1))),
                                                        k.lt(crd(k, f, 0, k.R(f)), N(k, "0"))))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* read {
            k.b.text(brd(k, f, 0, unframed ? k.v(i) : k.sub(k.v(i), N(k, "1"))), unframed ? "p[i]" : "p[i - 1]")};
        const Expr* clause {
            k.lor(unframed ? k.lt(k.v(i), k.pv(f, 1)) : k.eq_c(k.v(i), k.i32, "0"), k.ge_c(read, k.i32, "0"))};
        const Stmt* body {
            k.blkc({k.iff(k.lt(brd(k, f, 0, k.v(i)), N(k, "0")), k.blkc({k.ret(k.v(i))}), nullptr), inc(k, i)})};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), k.cl(clause)},
                                {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.decl(i, N(k, "0")), loop};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(k.ret(N(k, "-1")));
        f->body = k.blk(std::move(bs));
        k.done(f);
        if (vac_at != nullptr && c != nullptr)
        {
            *vac_at = c->loc;
        }
        return read;
    };
    lc("rngd_element_guarded",
       "p[i - 1] under i == 0 || in a loop invariant over reads_range(p, n): framed by the range for 0 < i <= n",
       [=](Kit& k)
       {
           first_negative(k, false, false);
       });
    lc("rngd_element_guarded_vacuity", "the same with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           SrcLoc at;
           first_negative(k, false, true, &at);
           k.fail("check", at);
       });
    lc("rngd_element_unframed",
       "i < n || p[i] >= 0 as an invariant: for i == n (and for n == 0 at the entry) the range place ends at n, the read is unframed (permission at the read in the clause: entry and back edge)",
       [=](Kit& k)
       {
           // two real cases at one line: n == 0 at the entry (p[0] is outside an empty range), and an iteration that reaches i == n; the driver prints them as one
           k.fail("permission", first_negative(k, true, false)->loc, 2U, "",
                  "this loop invariant reads `p[i]`, which this loop does not hold");
       });

    // int count(int *p, int n) requires(owns(*p) && 0 <= n && n <= 10 && *p >= 0) { int i = 0; while (i < n) inv 0 <= i <= n && p[<idx>] >= 0 decreases n - i { [*p = -1;] i++; } return i; }
    // `mode`: 0 p[0] framed by the cell place, 1 the body stores -1 (not preserved), 2 p[1] (no atom holds it)
    const auto cell_element = [=](Kit& k, int mode, SrcLoc* clause_at = nullptr) -> const Expr*
    {
        Function* f {k.fn("count", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()), k.ge_c(fx(k, f, 1), k.i32, "0")),
                        k.land(k.le_c(fx(k, f, 1), k.i32, "10"), k.ge_c(k.deref(fx(k, f, 0)), k.i32, "0"))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* read {k.b.text(brd(k, f, 0, N(k, mode == 2 ? "1" : "0")), mode == 2 ? "p[1]" : "p[0]")};
        const Expr* clause {k.ge_c(read, k.i32, "0")};
        std::vector<const Stmt*> bs;
        if (mode == 1)
        {
            bs.push_back(k.store(k.pn(f->param_var(0)), k.neg(N(k, "1"))));
        }
        bs.push_back(inc(k, i));
        const Clause kept {k.cl(clause)};
        if (clause_at != nullptr)
        {
            *clause_at = kept.loc;
        }
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), kept},
                                {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(k.v(i))});
        k.done(f);
        return read;
    };
    lc("rngd_element_cell",
       "p[0] >= 0 as an invariant under owns(*p): framed by the cell place of *p (the element is the cell)",
       [=](Kit& k)
       {
           cell_element(k, 0);
       });
    lc("rngd_element_cell_write",
       "the body stores -1 to *p: p[0] >= 0 is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           SrcLoc at;
           cell_element(k, 1, &at);
           k.fail("invariant-preserved", at);
       });
    lc("rngd_element_cell_unframed",
       "the invariant reads p[1], which no atom of the function holds: permission at the read (the entry assert fails, and its assumption ends the path)",
       [=](Kit& k)
       {
           k.fail_only("permission", cell_element(k, 2)->loc, 2U, 0U,
                       "this loop invariant reads `p[1]`, which this loop does not hold");
       });

    // the callers, with both guard values: int use_both(void) ensures(result == 0) { int buf[4] = {1, 2, 3, 4}; int r0 = beside(buf, 0); int r1 = beside(buf, 4); return r0 + r1; }
    const auto beside_callers = [=](Kit& k, bool vac) -> const Stmt*
    {
        const Function* bf {nullptr};
        beside(k, 0, &bf);
        Function* u {k.fn("use_both", k.i32)};
        k.ens(u, k.eq_c(k.R(u), k.i32, "0"));
        Var* buf {arr(k, u, "buf", k.i32, 4U)};
        Var* r0 {k.local(u, "r0", k.i32)};
        Var* r1 {k.local(u, "r1", k.i32)};
        std::vector<const Stmt*> st {k.decl(buf, nullptr)};
        for (int j {0}; j < 4; ++j)
        {
            st.push_back(awr(k, buf, N(k, std::to_string(j).c_str()), N(k, std::to_string(j + 1).c_str())));
        }
        st.push_back(k.decl(r0, k.call(bf, {aat(k, buf, N(k, "0")), N(k, "0")})));
        st.push_back(k.decl(r1, k.call(bf, {aat(k, buf, N(k, "0")), N(k, "4")})));
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        st.push_back(k.ret(k.add(k.v(r0), k.v(r1))));
        u->body = k.blkc(std::move(st));
        k.done(u);
        return c;
    };
    lc("rngd_beside_callers",
       "beside(buf, 0) (the guard false, nothing given) and beside(buf, 4) on a four-element array: both guard values, the array is whole again after each call",
       [=](Kit& k)
       {
           beside_callers(k, false);
       });
    lc("rngd_beside_callers_vacuity", "the same with check(false) before the return: it fails",
       [=](Kit& k)
       {
           vac_fail(k, beside_callers(k, true));
       });

    // ---- a callee in the body: it takes one element of the loop's range and gives it back (or keeps it) ----
    // void give(int *q) requires(owns(*q)) ensures(owns(*q) && *q == 5);   void keep(int *q) requires(owns(*q));
    // void fill5(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { give(&p[i]); [check(false);] i++; } }
    const auto callee_loop = [=](Kit& k, bool keeps, bool vac, SrcLoc* body_end = nullptr) -> const Stmt*
    {
        Function* g {k.fn(keeps ? "keep" : "give", k.vd, {{"q", PI(k)}})};
        k.req(g, k.b.atom(AtomKind::Owns, fx(k, g, 0), 0, k.at()));
        if (!keeps)
        {
            k.ens(g,
                  k.land(k.b.atom(AtomKind::Owns, fx(k, g, 0), 0, k.at()), k.eq_c(k.deref(fx(k, g, 0)), k.i32, "5")));
        }
        k.trusted(g);
        k.done(g);
        Function* f {k.fn("fill5", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, RO(k, f, 0, 1, 0));
        k.ens(f, RO(k, f, 0, 1, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.es(k.call(g, {bat(k, f, 0, k.v(i))}))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* body {k.blkc(bs)};
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        if (body_end != nullptr)
        {
            *body_end = body->end_loc;
        }
        return c;
    };
    lc("rngd_callee_gives",
       "give(&p[i]) takes one element of the loop's range and gives it back each iteration: the back-edge marked rows hold",
       [=](Kit& k)
       {
           callee_loop(k, false, false);
       });
    lc("rngd_callee_gives_vacuity", "the same with check(false) after the call in the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, callee_loop(k, false, true));
       });
    lc("rngd_callee_keeps",
       "keep(&p[i]) never gives the element back (nor the liveness fraction it took): the parameter's liveness row, the first back-edge row, fails at the end of the body (permission, the range's own message); its assumption is contradictory, so the marked rows after it report nothing",
       [=](Kit& k)
       {
           SrcLoc end;
           callee_loop(k, true, false, &end);
           k.fail_only("permission", end, 3U, 0U, "an iteration of this loop ends without the permission to `p[0..n)`");
       });

    // ---- a read range and a read cell in one loop: the cell place carries "not inside the range" ----
    // int count_eq(int *p, int n, int *q) requires(reads_range(p, n) && reads(*q)) ensures(reads_range(p, n) && reads(*q) && 0 <= result && result <= n)
    //   { int i = 0; int c = 0; while (i < n) inv 0 <= i <= n && 0 <= c && c <= [i|i - 1] decreases n - i { if (p[i] == *q) { c++; } i++; } return c; }
    // `entry`: the invariant claims c < i, false on entry
    const auto range_cell = [=](Kit& k, bool entry, bool vac, const Function** fout = nullptr,
                                SrcLoc* clause_at = nullptr) -> const Stmt*
    {
        Function* f {k.fn("count_eq", k.i32, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())));
        k.ens(f, k.land(k.land(RR(k, f, 0, 1, 0), k.b.atom(AtomKind::Reads, fx(k, f, 2), 1, k.at())),
                        k.land(k.ge_c(k.R(f), k.i32, "0"), k.le(k.R(f), fx(k, f, 1)))));
        Var* i {k.local(f, "i", k.i32)};
        Var* c {k.local(f, "c", k.i32)};
        const Stmt* body {k.blkc({k.iff(k.eq(brd(k, f, 0, k.v(i)), k.deref(k.pn(f->param_var(2)))),
                                        k.blkc({k.asg(c, k.add(k.v(c), N(k, "1")))}), nullptr),
                                  inc(k, i)})};
        const Clause cc {k.cl(k.land(k.ge_c(k.v(c), k.i32, "0"), entry ? k.lt(k.v(c), k.v(i)) : k.le(k.v(c), k.v(i))))};
        if (clause_at != nullptr)
        {
            *clause_at = cc.loc;
        }
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), cc}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        const Stmt* v {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.decl(i, N(k, "0")), k.decl(c, N(k, "0")), loop};
        if (v != nullptr)
        {
            bs.push_back(v);
        }
        bs.push_back(k.ret(k.v(c)));
        f->body = k.blk(std::move(bs));
        k.done(f);
        if (fout != nullptr)
        {
            *fout = f;
        }
        return v;
    };
    lc("rngd_range_cell",
       "a reads_range and a reads(*q) cell in one loop that reads both: the cell place carries the guard 'not inside the range', so the one function verifies whether q is inside the range or not",
       [=](Kit& k)
       {
           range_cell(k, false, false);
       });
    lc("rngd_range_cell_vacuity", "the same with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, range_cell(k, false, true));
       });
    lc("rngd_range_cell_entry",
       "the invariant claims c < i, false on entry (c == i == 0): invariant-entry at the clause",
       [=](Kit& k)
       {
           SrcLoc at;
           range_cell(k, true, false, nullptr, &at);
           k.fail("invariant-entry", at);
       });
    // int inside(void) { int a[3] = {1, 2, 3}; return count_eq(a, 3, &a[1]); }  and outside with a separate cell
    const auto range_cell_callers = [=](Kit& k, bool inside, bool vac) -> const Stmt*
    {
        const Function* cf {nullptr};
        range_cell(k, false, false, &cf);
        Function* u {k.fn(inside ? "inside" : "outside", k.i32)};
        Var* a {arr(k, u, "a", k.i32, 3U)};
        Var* x {k.cell(u, "x", k.i32)};
        std::vector<const Stmt*> st {k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")),
                                     awr(k, a, N(k, "1"), N(k, "2")), awr(k, a, N(k, "2"), N(k, "3")),
                                     k.decl(x, N(k, "2"))};
        Var* r {k.local(u, "r", k.i32)};
        st.push_back(
            k.decl(r, k.call(cf, {aat(k, a, N(k, "0")), N(k, "3"), inside ? aat(k, a, N(k, "1")) : k.addr(x)})));
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        st.push_back(k.ret(k.v(r)));
        u->body = k.blkc(std::move(st));
        k.done(u);
        return c;
    };
    lc("rngd_range_cell_inside", "count_eq(a, 3, &a[1]): the cell is inside the range (both reads), the call verifies",
       [=](Kit& k)
       {
           range_cell_callers(k, true, false);
       });
    lc("rngd_range_cell_inside_vacuity", "the same with check(false) before the return: it fails",
       [=](Kit& k)
       {
           vac_fail(k, range_cell_callers(k, true, true));
       });
    lc("rngd_range_cell_outside", "count_eq(a, 3, &x) with a separate cell: the call verifies",
       [=](Kit& k)
       {
           range_cell_callers(k, false, false);
       });
    lc("rngd_range_cell_outside_vacuity", "the same with check(false) before the return: it fails",
       [=](Kit& k)
       {
           vac_fail(k, range_cell_callers(k, false, true));
       });

    // ---- two read ranges that may share a slot ----
    // void two_reads(int *p, int n, int *q, int m) requires(reads_range(p, n) && reads_range(q, m) && m == n && n > 1) ensures(the same)
    //   { [keep_slot(&p[1]);] int i = 0; int x = 0; while (i < n) inv 0 <= i <= n decreases n - i { x = p[i]; [keep_slot(&p[1]);] x = q[i]; i++; } }
    // `mode`: 0 the loop, 1 a callee keeps a fraction of slot 1 before the loop (the entry rows), 2 it does so in the body (the back-edge rows)
    const auto two_reads = [=](Kit& k, int mode, bool vac, SrcLoc* loop_at = nullptr,
                               SrcLoc* body_end = nullptr) -> const Stmt*
    {
        Function* kf {nullptr};
        if (mode == 1 || mode == 2)
        {
            kf = k.fn("keep_slot", k.vd, {{"x", PI(k)}});
            k.req(kf, k.b.atom(AtomKind::Reads, fx(k, kf, 0), 0, k.at()));
            k.ens(kf, k.b.atom(AtomKind::Live, fx(k, kf, 0), 1, k.at()));
            kf->body = k.blk({});
            k.done(kf);
        }
        else if (mode == 3)
        {
            // void eat(int *x, int c) requires(reads_range(x, c)): bodiless, it keeps the fraction it takes of every slot
            kf = k.fn("eat", k.vd, {{"x", PI(k)}, {"c", k.i32}});
            k.req(kf, RR(k, kf, 0, 1, 0));
            k.trusted(kf);
            k.done(kf);
        }
        Function* f {k.fn("two_reads", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}})};
        const Expr* shape {k.land(k.eq(fx(k, f, 3), fx(k, f, 1)), k.gt(fx(k, f, 1), N(k, "1")))};
        if (mode == 3)
        {
            shape = k.land(shape, k.eq(fx(k, f, 2), fx(k, f, 0))); // p == q: every slot of the range is shared
        }
        k.req(f, k.land(k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)), shape));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)));
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        std::vector<const Stmt*> bs {k.asg(x, brd(k, f, 0, k.v(i)))};
        if (mode == 2)
        {
            bs.push_back(k.es(k.call(kf, {bat(k, f, 0, N(k, "1"))})));
        }
        bs.push_back(k.asg(x, brd(k, f, 2, k.v(i))));
        bs.push_back(inc(k, i));
        const Stmt* body {k.blkc(bs)};
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        std::vector<const Stmt*> st;
        if (mode == 1)
        {
            st.push_back(k.es(k.call(kf, {bat(k, f, 0, N(k, "1"))})));
        }
        if (mode == 3)
        {
            st.push_back(k.es(k.call(kf, {k.pn(f->param_var(0)), k.pv(f, 1)})));
        }
        st.push_back(k.decl(i, N(k, "0")));
        st.push_back(k.decl(x, N(k, "0")));
        st.push_back(loop);
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        f->body = k.blk(std::move(st));
        k.done(f);
        if (loop_at != nullptr)
        {
            *loop_at = loop->loc;
        }
        if (body_end != nullptr)
        {
            *body_end = body->end_loc;
        }
        return c;
    };
    lc("rngd_two_reads",
       "two reads_range places that may overlap, both read in the body, no exclusion of the earlier one: both marked rows take the sum form, and the function verifies",
       [=](Kit& k)
       {
           two_reads(k, 0, false);
       });
    lc("rngd_two_reads_vacuity", "the same with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, two_reads(k, 0, true));
       });
    lc("rngd_two_reads_keep_entry",
       "keep_slot(&p[1]) keeps a fraction of slot 1 before the loop (the callee takes half of what the slot holds and returns only the liveness): the loop begins without the permission to every element of p[0..n): the first marked entry row, p's value row, fails at the loop (permission); the failed marked row ends the path",
       [=](Kit& k)
       {
           SrcLoc at;
           two_reads(k, 1, false, &at);
           k.fail_only("permission", at, 2U, 0U,
                       "this loop begins without the permission to every element of `p[0..n)`");
       });
    lc("rngd_two_reads_keep_body",
       "keep_slot(&p[1]) in the body keeps a fraction of slot 1 and the liveness fraction it takes does not come back: the end of the body fails twice, the parameter's liveness row and then the first marked back-edge row, both permission with the range's message",
       [=](Kit& k)
       {
           SrcLoc end;
           two_reads(k, 2, false, nullptr, &end);
           k.fail_only("permission", end, 3U, 0U, "an iteration of this loop ends without the permission to `p[0..n)`");
           k.fail_only("permission", end, 3U, 1U, "an iteration of this loop ends without the permission to `p[0..n)`");
       });
    lc("rngd_two_reads_eat_entry",
       "p == q, and eat(p, n) (reads_range, bodiless: it keeps the fraction it takes of every slot, a third of what each holds) before the loop: every slot is shared, each place's own amount is still there, their sum is not: the first sum-form marked entry row fails at the loop (permission); with per-place rows only, Silicon's own invariant check fails on entry (an internal reason, an exit 3)",
       [=](Kit& k)
       {
           SrcLoc at;
           two_reads(k, 3, false, &at);
           k.fail_only("permission", at, 2U, 0U,
                       "this loop begins without the permission to every element of `p[0..n)`");
       });

    // ---- the assigned parameter: no range place for it (question 237's pinned false alarm) ----
    // int pin(int *p, int *q, int n) requires(reads_range(p, n) && reads_range(q, n) && n > 0) ensures(reads_range(p, n) && reads_range(q, n))
    //   { int i = 0; int z = 0; p = q; while (i < n) inv 0 <= i <= n && 0 <= z && z <= i decreases n - i { if (q[i] == 0) { z++; } i++; } return z; }
    // The loop reads q only: p's range is not carried by the loop, it stays in the frame and comes back after it: the program verifies (no false alarm here).
    const auto pin = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("pin", k.i32, {{"p", PI(k)}, {"q", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)), k.gt(fx(k, f, 2), N(k, "0"))));
        k.ens(f, k.land(RR(k, f, 0, 2, 0), RR(k, f, 1, 2, 1)));
        Var* i {k.local(f, "i", k.i32)};
        Var* z {k.local(f, "z", k.i32)};
        const Stmt* body {k.blkc(
            {k.iff(k.eq_c(brd(k, f, 1, k.v(i)), k.i32, "0"), k.blkc({k.asg(z, k.add(k.v(z), N(k, "1")))}), nullptr),
             inc(k, i)})};
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 2)),
                  {k.cl(k.land(bounds(k, f, i, 2), k.land(k.ge_c(k.v(z), k.i32, "0"), k.le(k.v(z), k.v(i)))))},
                  {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, body)};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> st {k.decl(i, N(k, "0")), k.decl(z, N(k, "0")),
                                     k.asg(f->param_var(0), k.pn(f->param_var(1))), loop};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        st.push_back(k.ret(k.v(z)));
        f->body = k.blk(std::move(st));
        k.done(f);
        return c;
    };
    lc("rngd_assigned_param_frame",
       "the function assigns p (p = q) and the loop reads only q: p has no range place, so its range stays in the frame through the loop and comes back whole: the program verifies",
       [=](Kit& k)
       {
           pin(k, false);
       });
    lc("rngd_assigned_param_frame_vacuity", "the same with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, pin(k, true));
       });
    // int pin2(int *p, int n) requires(reads_range(p, n) && n > 1) ensures(reads_range(p, n))
    //   { int i = 0; int z = 0; p = &p[1]; while (i < n - 1) inv 0 <= i <= n - 1 && 0 <= z && z <= i decreases n - 1 - i { if (p[i] == 0) { z++; } i++; } return z; }
    // The loop reads slots of p's range through the assigned p: nothing carries them in the loop (a pinned false alarm, question 237)
    const auto pin_read = [=](Kit& k, SrcLoc* read_at) -> const Stmt*
    {
        Function* f {k.fn("pin2", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, RR(k, f, 0, 1, 0));
        Var* i {k.local(f, "i", k.i32)};
        Var* z {k.local(f, "z", k.i32)};
        const Expr* rd {k.b.text(brd(k, f, 0, k.v(i)), "p[i]")};
        *read_at = rd->loc;
        const Expr* lim {k.sub(k.pv(f, 1), N(k, "1"))};
        const Stmt* body {
            k.blkc({k.iff(k.eq_c(rd, k.i32, "0"), k.blkc({k.asg(z, k.add(k.v(z), N(k, "1")))}), nullptr), inc(k, i)})};
        const Stmt* loop {k.whl(k.lt(k.v(i), lim),
                                {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"),
                                             k.land(k.le(k.v(i), k.sub(k.pv(f, 1), N(k, "1"))),
                                                    k.land(k.ge_c(k.v(z), k.i32, "0"), k.le(k.v(z), k.v(i))))))},
                                {k.cl(k.sub(k.sub(k.pv(f, 1), N(k, "1")), k.v(i)))}, body)};
        f->body = k.blk({k.decl(i, N(k, "0")), k.decl(z, N(k, "0")), k.asg(f->param_var(0), bat(k, f, 0, N(k, "1"))),
                         loop, k.ret(k.v(z))});
        k.done(f);
        return nullptr;
    };
    lc("rngd_assigned_param",
       "the function assigns p (p = &p[1]) and the loop reads p[i] through it: p has no range place and the loop carries nothing for those slots, so the read is a user error (a false alarm: the program is correct, pinned, question 237); the errors are the read's own rows, never an internal one",
       [=](Kit& k)
       {
           SrcLoc at;
           pin_read(k, &at);
           k.fail("lifetime", at);
       });

    // ---- a loop that assigns the count parameter: the range's bounds are the entry values ----
    // void dec(int *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n)) { while (n > 0) inv 0 <= n decreases n { p[0] = 5; n = n - 1; } }
    const auto dec_count = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("dec", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, RO(k, f, 0, 1, 0));
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {bwr(k, f, 0, N(k, "0"), N(k, "5")),
                                     k.asg(f->param_var(1), k.sub(k.pv(f, 1), N(k, "1")))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        f->body = k.blk({k.whl(k.gt(k.pv(f, 1), N(k, "0")), {k.cl(k.ge_c(k.pv(f, 1), k.i32, "0"))}, {k.cl(k.pv(f, 1))},
                               k.blkc(bs))});
        k.done(f);
        return c;
    };
    lc("rngd_count_assigned",
       "the loop decrements the count parameter n: the range place keeps the entry value of n (the formal), so p[0] stays inside it and the range comes back whole",
       [=](Kit& k)
       {
           dec_count(k, false);
       });
    lc("rngd_count_assigned_body_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, dec_count(k, true));
       });

    // ---- an array used as a pointer in an invariant ----
    // int walk(void) { int a[3] = {1, 2, 3}; int *q = &a[0]; int i = 0; while (i < 3) inv 0 <= i <= 3 && q == a decreases 3 - i { [q = &a[1];] i++; } return *q; }
    const auto walk = [=](Kit& k, bool moved, bool vac, SrcLoc* clause_at = nullptr) -> const Stmt*
    {
        Function* f {k.fn("walk", k.i32)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* q {k.local(f, "q", PI(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Clause same {k.cl(k.eq(k.v(q), aat(k, a, N(k, "0"))))};
        if (clause_at != nullptr)
        {
            *clause_at = same.loc;
        }
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs;
        if (moved)
        {
            bs.push_back(k.asg(q, aat(k, a, N(k, "1"))));
        }
        bs.push_back(inc(k, i));
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), same},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs))};
        f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                          awr(k, a, N(k, "2"), N(k, "3")), k.decl(q, aat(k, a, N(k, "0"))), k.decl(i, N(k, "0")), loop,
                          k.ret(k.deref(k.pn(q)))});
        k.done(f);
        return c;
    };
    lc("rngd_array_as_pointer",
       "q == a in a loop invariant (the decay of an array local is the address of its element 0): q keeps the address of a[0] through the loop",
       [=](Kit& k)
       {
           walk(k, false, false);
       });
    lc("rngd_array_as_pointer_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, walk(k, false, true));
       });
    lc("rngd_array_as_pointer_moved",
       "the body moves q to &a[1]: q == a is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           SrcLoc at;
           walk(k, true, false, &at);
           k.fail("invariant-preserved", at);
       });

    // ---- nested loops over one range ----
    // void nest(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { int j = 0; while (j < n) inv 0 <= j <= n decreases n - j { p[j] = i; j++; } i++; } }
    const auto nest = [=](Kit& k, bool vac) -> const Stmt*
    {
        Function* f {k.fn("nest", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, RO(k, f, 0, 1, 0));
        k.ens(f, RO(k, f, 0, 1, 0));
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Stmt* inner {k.whl(k.lt(k.v(j), k.pv(f, 1)), {k.cl(bounds(k, f, j, 1))},
                                 {k.cl(k.sub(k.pv(f, 1), k.v(j)))}, k.blkc({bwr(k, f, 0, k.v(j), k.v(i)), inc(k, j)}))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ob {k.decl(j, N(k, "0")), inner};
        if (c != nullptr)
        {
            ob.push_back(c);
        }
        ob.push_back(inc(k, i));
        const Stmt* outer {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(ob))};
        f->body = k.blk({k.decl(i, N(k, "0")), outer});
        k.done(f);
        return c;
    };
    lc("rngd_nested", "two nested loops over one owned range: each carries the range place, its own label and rows",
       [=](Kit& k)
       {
           nest(k, false);
       });
    lc("rngd_nested_vacuity", "the same with check(false) after the inner loop: it fails",
       [=](Kit& k)
       {
           vac_fail(k, nest(k, true));
       });

    // ---- a read range and a global's cell in one loop: the global's place carries "not inside the range" ----
    // int g;   void gl(int *p, int n) requires(reads_range(p, n) && reads(g)) ensures(the same)
    //   { int i = 0; int x = 0; int y = 0; while (i < n) inv 0 <= i <= n decreases n - i { x = p[i]; y = g; [check(false);] i++; } }
    // int main() { int a[4] = {1, 2, 3, 4}; gl(<&g, 1 | &a[0], 4>); return 0; }   (g inside the range, or beside it)
    // `mode`: 0 the callee alone, 1 the caller with g inside the range, 2 the caller with a separate range; `vac` the vacuity twin (the end of the body, or after the call)
    const auto global_cell = [=](Kit& k, int mode, bool vac) -> const Stmt*
    {
        Global* g {k.prog.add_global("g", k.i32, k.at(), nullptr)};
        g->init = N(k, "3");
        g->value = "3";
        auto gatom = [&](int amount)
        {
            return k.b.global_atom(AtomKind::Reads, k.b.global_addr(g, k.at()), amount, k.at());
        };
        Function* f {k.fn("gl", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), gatom(1)));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), gatom(1)));
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        Var* y {k.local(f, "y", k.i32)};
        const Stmt* c {vac && mode == 0 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.asg(x, brd(k, f, 0, k.v(i))), k.asg(y, k.deref(k.b.global_addr(g, k.at())))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        f->body = k.blk({k.decl(i, N(k, "0")), k.decl(x, N(k, "0")), k.decl(y, N(k, "0")),
                         k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                               k.blkc(bs))});
        k.done(f);
        if (mode == 0)
        {
            return c;
        }
        Function* m {k.fn("main", k.i32)};
        Var* a {arr(k, m, "a", k.i32, 4U)};
        const Expr* call {mode == 1 ? k.call(f, {k.b.global_addr(g, k.at()), N(k, "1")})
                                    : k.call(f, {aat(k, a, N(k, "0")), N(k, "4")})};
        std::vector<const Stmt*> st {k.decl(a, nullptr),
                                     awr(k, a, N(k, "0"), N(k, "1")),
                                     awr(k, a, N(k, "1"), N(k, "2")),
                                     awr(k, a, N(k, "2"), N(k, "3")),
                                     awr(k, a, N(k, "3"), N(k, "4")),
                                     k.es(call)};
        const Stmt* v {vac ? k.chk(k.no()) : nullptr};
        if (v != nullptr)
        {
            st.push_back(v);
        }
        st.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(std::move(st));
        k.done(m);
        k.prog.set_entry(m);
        return v;
    };
    lc("rngd_global_cell",
       "a reads_range and reads(g) of a global in one loop that reads both: the global's place stands where g is not inside the range",
       [=](Kit& k)
       {
           global_cell(k, 0, false);
       });
    lc("rngd_global_cell_body_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, global_cell(k, 0, true));
       });
    lc("rngd_global_cell_inside", "gl(&g, 1): the global is the range's one cell (both reads), the call verifies",
       [=](Kit& k)
       {
           global_cell(k, 1, false);
       });
    lc("rngd_global_cell_inside_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           vac_fail(k, global_cell(k, 1, true));
       });
    lc("rngd_global_cell_outside",
       "gl(&a[0], 4) with a local array: the global is outside the range, the call verifies",
       [=](Kit& k)
       {
           global_cell(k, 2, false);
       });
    lc("rngd_global_cell_outside_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           vac_fail(k, global_cell(k, 2, true));
       });

    // ---- p[e] through a pointer into a local array, in an invariant: framed by the array's place ----
    // int qarr(int k) requires(0 <= k && k <= 3) { int a[3] = {1, 2, 3}; int *q = &a[<0|k>]; int i = 0; while (i < 3) inv 0 <= i <= 3 && q[0] >= 0 decreases 3 - i { i++; [check(false);] } return 0; }
    const auto array_through_pointer = [=](Kit& k, bool outside, bool vac, SrcLoc* read_at = nullptr) -> const Stmt*
    {
        Function* f {k.fn("qarr", k.i32, {{"k", k.i32}})};
        k.req(f, k.land(k.ge_c(fx(k, f, 0), k.i32, "0"), k.le_c(fx(k, f, 0), k.i32, "3")));
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* q {k.local(f, "q", PI(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* rd {k.b.text(k.deref(k.b.elem_addr(k.pn(q), N(k, "0"), k.at())), "q[0]")};
        if (read_at != nullptr)
        {
            *read_at = rd->loc;
        }
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {inc(k, i)};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")),
                                {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(k.ge_c(rd, k.i32, "0"))},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs))};
        f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                          awr(k, a, N(k, "2"), N(k, "3")), k.decl(q, aat(k, a, outside ? k.pv(f, 0) : N(k, "0"))),
                          k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return c;
    };
    lc("rngd_array_through_pointer",
       "q[0] >= 0 in a loop invariant with int *q = &a[0] of a local array the loop carries: the read is framed by the array's place (its block and 0 <= offset < 3) and the values are kept",
       [=](Kit& k)
       {
           array_through_pointer(k, false, false);
       });
    lc("rngd_array_through_pointer_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, array_through_pointer(k, false, true));
       });
    lc("rngd_array_through_pointer_outside",
       "the same with q = &a[k] for 0 <= k <= 3: for k == 3 q is one past the array and the read is unframed (permission at the read)",
       [=](Kit& k)
       {
           SrcLoc at;
           array_through_pointer(k, true, false, &at);
           k.fail_only("permission", at, 2U, 0U, "this loop invariant reads `q[0]`, which this loop does not hold");
       });

    // ---- the address of an element as a value in an invariant and a measure (compared with == and !=); in a contract it stays refused (range_rej_elem_address_value, arr_rej_elem_addr_in_*) ----
    // int adr(void) { int a[3] = {1, 2, 3}; int *q = &a[1]; int i = 0;
    //   while (i < 3) inv 0 <= i <= 3 && q == &a[1] && q != &a[2] decreases (q == &a[1] ? 3 : 3) - i { [q = &a[2];] i++; } return 0; }
    const auto address_clause = [=](Kit& k, bool moved, bool vac, SrcLoc* clause_at = nullptr) -> const Stmt*
    {
        Function* f {k.fn("adr", k.i32)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* q {k.local(f, "q", PI(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Clause same {k.cl(k.land(k.eq(k.v(q), aat(k, a, N(k, "1"))), k.ne(k.v(q), aat(k, a, N(k, "2")))))};
        if (clause_at != nullptr)
        {
            *clause_at = same.loc;
        }
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs;
        if (moved)
        {
            bs.push_back(k.asg(q, aat(k, a, N(k, "2"))));
        }
        bs.push_back(inc(k, i));
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        const Expr* measure {k.sub(k.cond(k.eq(k.v(q), aat(k, a, N(k, "1"))), N(k, "3"), N(k, "3")), k.v(i))};
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), same},
                                {k.cl(measure)}, k.blkc(bs))};
        f->body = k.blkc({k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                          awr(k, a, N(k, "2"), N(k, "3")), k.decl(q, aat(k, a, N(k, "1"))), k.decl(i, N(k, "0")), loop,
                          k.ret(N(k, "0"))});
        k.done(f);
        return c;
    };
    lc("rngd_address_clauses",
       "q == &a[1] && q != &a[2] in a loop invariant and the same address in the loop's measure: the address of an element is a value there, the loop keeps q",
       [=](Kit& k)
       {
           address_clause(k, false, false);
       });
    lc("rngd_address_clauses_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac_fail(k, address_clause(k, false, true));
       });
    lc("rngd_address_clauses_moved",
       "the body moves q to &a[2]: q == &a[1] is not preserved (invariant-preserved at the clause)",
       [=](Kit& k)
       {
           SrcLoc at;
           address_clause(k, true, false, &at);
           k.fail("invariant-preserved", at);
       });
}

// ---- Part E: markers over a range, and `restrict` backed by a range (record 0060 items 12 and 14) ----
// `must_init` and `ref_to_uninit` on a parameter whose `requires` atom is `owns_range(p, n)` (decision 0042 items 4-8 over a range): the `requires` atom has no flags conjunct and
// states the type's range under the flag; `must_init`'s `ensures` atom is the plain one and the flags row at a return is the promise (`postcondition`, "returns without initializing every
// element of `p[0..3)`, which `cerastes::must_init` on `p` promises"); `ref_to_uninit`'s `ensures` atom keeps a flag that was set (`old(flag) ==> flag`, an `internal` row at the return);
// a call has no flags row for a marked parameter.  `owns_range(p, n)` unguarded or after `p == NULL ||`, in both lists, backs `restrict`.
void define_range_markers()
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
    // `owns_range(p, <count>)` over a constant count, with its spelling
    auto RO3 = [=](Kit& k, Function* f, int p, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), k.b.text(N(k, "3"), "3"), amount, k.at());
    };
    auto ROn = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto RRn = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto bat = [](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.b.elem_addr(k.pn(f->param_var(p)), idx, k.at());
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
    auto vac_fail = [](Kit& k, const Stmt* c)
    {
        k.fail("check", c->loc);
    };

    // ---- a straight-line `must_init` filler ----
    // void fill3(int *p) [must_init p] requires(owns_range(p, 3)) ensures(owns_range(p, 3)) { p[0] = 1; p[1] = 2; [p[2] = 3;] [check(false);] }
    // `written`: the elements it writes (3: all, 2: leaves p[2]); `vac`: check(false) after the writes
    const auto filler = [=](Kit& k, int written, bool vac, SrcLoc* end = nullptr) -> const Function*
    {
        Function* f {k.fn("fill3", k.vd, {{"p", PI(k)}})};
        f->params[0U].init = InitMarker::MustInit;
        k.req(f, RO3(k, f, 0, 0));
        k.ens(f, RO3(k, f, 0, 0));
        std::vector<const Stmt*> st;
        for (int j {0}; j < written; ++j)
        {
            st.push_back(k.store(bat(k, f, 0, N(k, std::to_string(j).c_str())), N(k, std::to_string(j + 1).c_str())));
        }
        if (vac)
        {
            vac_fail(k, st.emplace_back(k.chk(k.no())));
        }
        f->body = k.blk(std::move(st));
        k.done(f);
        if (end != nullptr)
        {
            *end = f->end_loc;
        }
        return f;
    };
    lc("rngm_filler",
       "a must_init filler writes p[0], p[1], p[2] of owns_range(p, 3): the flags row at the return holds, no flag is assumed on entry",
       [=](Kit& k)
       {
           filler(k, 3, false);
       });
    lc("rngm_filler_vacuity", "the same with check(false) after the writes: it fails",
       [=](Kit& k)
       {
           filler(k, 3, true);
       });
    lc("rngm_filler_leaves_one",
       "the filler writes only p[0] and p[1]: its flags row at the return fails, postcondition, 'returns without initializing every element of `p[0..3)`, which `cerastes::must_init` on `p` promises'",
       [=](Kit& k)
       {
           SrcLoc end;
           filler(k, 2, false, &end);
           k.fail(
               "postcondition", end, 1U, "",
               "returns without initializing every element of `p[0..3)`, which `cerastes::must_init` on `p` promises");
       });
    // int user(void) { int a[3]; fill3(&a[0]); int x = a[1]; return x; }   a caller with an uninit array: the call has no flags row, and every element has a value after it
    const auto filler_caller = [=](Kit& k, bool vac) -> const Stmt*
    {
        const Function* ff {filler(k, 3, false)};
        Function* u {k.fn("user", k.i32)};
        Var* a {arr(k, u, "a", k.i32, 3U)};
        Var* x {k.local(u, "x", k.i32)};
        std::vector<const Stmt*> st {k.decl(a, nullptr), k.es(k.call(ff, {aat(k, a, N(k, "0"))})),
                                     k.decl(x, ard(k, a, N(k, "1")))};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        st.push_back(k.ret(k.v(x)));
        u->body = k.blkc(std::move(st));
        k.done(u);
        return c;
    };
    lc("rngm_filler_caller",
       "fill3(&a[0]) on an array no element of which was written: the call has no flags row, and a[1] is readable after it (the filler promised every element)",
       [=](Kit& k)
       {
           filler_caller(k, false);
       });
    lc("rngm_filler_caller_vacuity", "the same with check(false) before the return: it fails",
       [=](Kit& k)
       {
           vac_fail(k, filler_caller(k, true));
       });

    // ---- `ref_to_uninit` ----
    // void wr(int *p, int n) [ref_to_uninit p] requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n)) { [int x = p[0];] p[0] = 1; [check(false);] }
    // `mode`: 0 writes p[0], 1 reads p[0] before the write (no value to read), 2 check(false) after the write
    const auto ref_callee = [=](Kit& k, int mode, SrcLoc* read_at = nullptr) -> const Stmt*
    {
        Function* f {k.fn("wr", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        f->params[0U].init = InitMarker::RefToUninit;
        k.req(f, k.land(ROn(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, ROn(k, f, 0, 1, 0));
        Var* x {k.local(f, "x", k.i32)};
        const Expr* rd {k.b.text(k.deref(bat(k, f, 0, N(k, "0"))), "p[0]")};
        std::vector<const Stmt*> st;
        if (mode == 1)
        {
            st.push_back(k.decl(x, rd));
            if (read_at != nullptr)
            {
                *read_at = rd->loc;
            }
        }
        st.push_back(k.store(bat(k, f, 0, N(k, "0")), N(k, "1")));
        const Stmt* c {mode == 2 ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        f->body = k.blk(std::move(st));
        k.done(f);
        return mode == 1 ? static_cast<const Stmt*>(nullptr) : c;
    };
    lc("rngm_ref_callee",
       "a ref_to_uninit callee writes p[0] of owns_range(p, n): no flags on entry, the flags it had come back (old(flag) ==> flag), the range under the flag",
       [=](Kit& k)
       {
           ref_callee(k, 0);
       });
    lc("rngm_ref_callee_vacuity", "the same with check(false) after the write: it fails",
       [=](Kit& k)
       {
           vac_fail(k, ref_callee(k, 2));
       });
    lc("rngm_ref_read_first",
       "the callee reads p[0] before it writes it: nothing says the element has a value, uninitialized at the read",
       [=](Kit& k)
       {
           SrcLoc at;
           ref_callee(k, 1, &at);
           k.fail("uninitialized", at);
       });
    // int use_ref(void) { int a[3]; [a[0] = 7;] wr(&a[0], 3); int y = a[<0|1>]; [check(false);] return y; }
    // `mode`: 0 a[0] written before the call and read after (the flag it had is kept), 1 nothing written and a[1] read after (it never had a value), 2 mode 0 with check(false)
    const auto ref_caller = [=](Kit& k, int mode) -> const Expr*
    {
        Function* wf {nullptr};
        {
            wf = k.fn("wr", k.vd, {{"p", PI(k)}, {"n", k.i32}});
            wf->params[0U].init = InitMarker::RefToUninit;
            k.req(wf, k.land(ROn(k, wf, 0, 1, 0), k.gt(fx(k, wf, 1), N(k, "0"))));
            k.ens(wf, ROn(k, wf, 0, 1, 0));
            wf->body = k.blk({k.store(bat(k, wf, 0, N(k, "0")), N(k, "1"))});
            k.done(wf);
        }
        Function* u {k.fn("use_ref", k.i32)};
        Var* a {arr(k, u, "a", k.i32, 3U)};
        Var* y {k.local(u, "y", k.i32)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        if (mode != 1)
        {
            st.push_back(awr(k, a, N(k, "0"), N(k, "7")));
        }
        st.push_back(k.es(k.call(wf, {aat(k, a, N(k, "0")), N(k, "3")})));
        const Expr* rd {k.b.text(ard(k, a, N(k, mode == 1 ? "1" : "0")), mode == 1 ? "a[1]" : "a[0]")};
        st.push_back(k.decl(y, rd));
        if (mode == 2)
        {
            vac_fail(k, st.emplace_back(k.chk(k.no())));
        }
        st.push_back(k.ret(k.v(y)));
        u->body = k.blkc(std::move(st));
        k.done(u);
        return rd;
    };
    lc("rngm_ref_caller_kept",
       "a[0] is written, wr(&a[0], 3) is called on the array, a[0] is read after: the flag it had is kept",
       [=](Kit& k)
       {
           ref_caller(k, 0);
       });
    lc("rngm_ref_caller_vacuity", "the same with check(false) before the return: it fails",
       [=](Kit& k)
       {
           ref_caller(k, 2);
       });
    lc("rngm_ref_caller_unwritten",
       "nothing is written before wr(&a[0], 3) (the call passes: no flags row) and a[1] is read after it: it never had a value, uninitialized at the read",
       [=](Kit& k)
       {
           k.fail("uninitialized", ref_caller(k, 1)->loc);
       });

    // ---- `restrict` backed by a range ----
    // int rg(int *restrict p, int n) requires([p == NULL ||] owns_range(p, n)) ensures([p == NULL ||] owns_range(p, n)) { [check(false);] return 0; }
    const auto restrict_range = [=](Kit& k, bool guarded, bool vac) -> const Stmt*
    {
        Function* f {k.fn("rg", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        f->params[0U].is_restrict = true;
        auto atom = [&](int amount)
        {
            const Expr* a {ROn(k, f, 0, 1, amount)};
            return guarded ? k.lor(k.eq(fx(k, f, 0), k.nul(PI(k))), a) : a;
        };
        k.req(f, atom(0));
        k.ens(f, atom(0));
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> st;
        if (c != nullptr)
        {
            st.push_back(c);
        }
        st.push_back(k.ret(N(k, "0")));
        f->body = k.blk(std::move(st));
        k.done(f);
        return c;
    };
    lc("rngm_restrict_range", "int *restrict p backed by owns_range(p, n) in both lists",
       [=](Kit& k)
       {
           restrict_range(k, false, false);
       });
    lc("rngm_restrict_range_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vac_fail(k, restrict_range(k, false, true));
       });
    lc("rngm_restrict_range_null", "the atom after p == NULL ||, in both lists",
       [=](Kit& k)
       {
           restrict_range(k, true, false);
       });
    lc("rngm_restrict_range_null_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vac_fail(k, restrict_range(k, true, true));
       });
    // int rq(int *restrict p, int n, int *q, int m) requires(owns_range(p, n) && reads_range(q, m)) ensures(the same): another parameter of the pointee type has a range atom of its own
    lc("rngm_restrict_range_other_covered", "another int * parameter covered by a reads_range of its own: accepted",
       [=](Kit& k)
       {
           Function* f {k.fn("rq", k.i32, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.land(ROn(k, f, 0, 1, 0), RRn(k, f, 2, 3, 1)));
           k.ens(f, k.land(ROn(k, f, 0, 1, 0), RRn(k, f, 2, 3, 1)));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("rngm_rej_restrict_reads_range", "restrict",
       [=](Kit& k)
       {
           // a reads_range does not back restrict: nothing is owned
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].is_restrict = true;
           k.req(f, RRn(k, f, 0, 1, 0));
           k.ens(f, RRn(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("rngm_rej_restrict_range_guard", "restrict",
       [=](Kit& k)
       {
           // n == 0 || owns_range(p, n): a guard that is not `p == NULL` does not back restrict
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), ROn(k, f, 0, 1, 0)));
           k.ens(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), ROn(k, f, 0, 1, 0)));
           f->body = k.blk({});
           k.done(f);
       });
    rc("rngm_rej_restrict_range_other_uncovered", "restrict",
       [=](Kit& k)
       {
           // another int * parameter with no atom of its own could alias the restrict pointer's elements
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
           f->params[0U].is_restrict = true;
           k.req(f, ROn(k, f, 0, 1, 0));
           k.ens(f, ROn(k, f, 0, 1, 0));
           f->body = k.blk({});
           k.done(f);
       });
}

// ---- Part C: the caller (record 0060 items 6, 8 and 10) ----
// A call of a function that holds range atoms: the base and the count of each atom are bound to locals (no row reads the heap for them), then per atom the shape row first (a
// null pointer, a negative count and a range too long for its object end the path there: one error), a `reads_range`'s first-slot row and the liveness row, then the separation
// rows (a range and a cell, two ranges), the amounts (`owns_range`: the liveness cell's; `reads_range`: the least of the first slot's cells, the liveness cell and the caller's own
// read amounts of the field, each divided by N + 1 and bound to a `Perm` local), per atom the marked rows (inside `if (g) { }`), the flags (`uninitialized`) and the type's range
// under the flag, and the sums.  Twins fail with one error at the call, in the order built (the S44 (b), (c), (d) and (i) fixtures).
void define_range_calls()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.b.text(k.num(k.i32, v),
                        v); // with its spelling, so that a call's messages name the range as written
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto RO = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int p, int n, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto crd = [=](Kit& k, Function* f, int p, const Expr* idx)
    {
        return k.deref(k.b.elem_addr(fx(k, f, p), idx, k.at()));
    };
    auto arr = [](Kit& k, Function* f, const char* name, Type element, unsigned long long count)
    {
        Var* x {f->new_local(name, k.prog.array_of(element, count), k.at())};
        f->mark_address_taken(x);
        return x;
    };
    // `&a[i]`, `a[i]` and `a[i] = v;` of a named array
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
    auto vacuity_c = [](Kit& k, const Stmt* c)
    {
        k.fail("check", c->loc);
    };
    // a positive case and its vacuity twin: the same program with `check(false)` last, which must fail
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           std::string(
               "the same with check(false) after everything: it fails (the path through the contract is not empty)"),
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    // ---- the callees ----
    // void touch(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) {}
    auto mk_touch = [=](Kit& k)
    {
        Function* f {k.fn("touch", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, RO(k, f, 0, 1, 0));
        k.ens(f, RO(k, f, 0, 1, 0));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    // void zero_first(int *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n) && p[0] == 0) { p[0] = 0; }
    auto mk_zero_first = [=](Kit& k)
    {
        Function* f {k.fn("zero_first", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), k.eq(crd(k, f, 0, N(k, "0")), N(k, "0"))));
        f->body = k.blk({k.store(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "0"), k.at()), N(k, "0"))});
        k.done(f);
        return f;
    };
    // void rdr(int *p, int n) requires(reads_range(p, n)) ensures(reads_range(p, n)) {}
    auto mk_rdr = [=](Kit& k)
    {
        Function* f {k.fn("rdr", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0, 1, 0));
        k.ens(f, RR(k, f, 0, 1, 0));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    // void two(int *p, int n, int *q, int m) requires(owns_range(p, n) && owns_range(q, m)) ensures(the same) {}   and bothr with reads_range
    auto mk_pair = [=](Kit& k, bool reads)
    {
        Function* f {k.fn(reads ? "bothr" : "two", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}})};
        auto at = [&](int p, int n, int amount)
        {
            return reads ? RR(k, f, p, n, amount) : RO(k, f, p, n, amount);
        };
        k.req(f, k.land(at(0, 1, 0), at(2, 3, 1)));
        k.ens(f, k.land(at(0, 1, 0), at(2, 3, 1)));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    // a caller `void c() { int a[4]; a[0] = 1; ... (`written` elements); <body> }`; the body's statements are made by `body`
    auto caller = [=](Kit& k, int written, const std::function<std::vector<const Stmt*>(Kit&, Function*, Var*)>& body)
    {
        Function* f {k.fn("c", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 4U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr)};
        for (int i {0}; i < written; ++i)
        {
            st.push_back(awr(k, a, N(k, std::to_string(i).c_str()), N(k, std::to_string(i + 1).c_str())));
        }
        for (const Stmt* x : body(k, f, a))
        {
            st.push_back(x);
        }
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        f->body = k.blkc(std::move(st));
        k.done(f);
        return f;
    };
    // `touch(&a[<off>], <count>)` and the call expression, for the failure's position
    auto call_touch = [=](Kit& k, Function* callee, Var* a, const char* off, const char* count, const Expr** out)
    {
        const Expr* c {k.call(callee, {aat(k, a, N(k, off)), N(k, count)})};
        *out = c;
        return k.es(c);
    };

    // ---- S44 (b): the caller on a local array ----
    // whole array, then a check of the value the callee promises; twin: a value the callee did not promise
    const auto whole = [=](Kit& k, bool lost, bool vac) -> const Stmt*
    {
        Function* zf {mk_zero_first(k)};
        const Stmt* c {nullptr};
        caller(k, 4,
               [&](Kit& kk, Function*, Var* a)
               {
                   const Expr* call {nullptr};
                   const Stmt* cs {call_touch(kk, zf, a, "0", "4", &call)};
                   c = kk.chk(vac ? kk.no()
                                  : (lost ? kk.eq_c(ard(kk, a, N(kk, "1")), kk.i32, "2")
                                          : kk.eq_c(ard(kk, a, N(kk, "0")), kk.i32, "0")));
                   return std::vector<const Stmt*> {cs, c};
               });
        return c;
    };
    lc("rngc_whole",
       "int a[4] written, zero_first(&a[0], 4): a[0] == 0 afterwards, the range comes back whole (the array's scope exit holds)",
       [=](Kit& k)
       {
           whole(k, false, false);
       });
    lc("rngc_whole_vacuity", "the same with check(false): it fails",
       [=](Kit& k)
       {
           vacuity_c(k, whole(k, false, true));
       });
    lc("rngc_values_lost",
       "the same checking a[1] == 2: an owning callee may have written any element but the first it promises: the check fails",
       [=](Kit& k)
       {
           k.fail("check", whole(k, true, false)->loc);
       });

    // a call `touch(<args>)` on a four-element array of which `written` elements are written, with the call's expression for the failure's position
    const auto touch_at = [=](Kit& k, int written, const char* off, const char* count, bool null_base) -> const Expr*
    {
        Function* tf {mk_touch(k)};
        const Expr* call {nullptr};
        caller(k, written,
               [&](Kit& kk, Function*, Var* a)
               {
                   call = kk.call(tf, {null_base ? kk.nul(PI(kk)) : aat(kk, a, N(kk, off)), N(kk, count)});
                   return std::vector<const Stmt*> {kk.es(call)};
               });
        return call;
    };
    lcv("rngc_sub_range",
        "touch(&a[1], 2) on a written four-element array: the range is a sub-range, the rest is in the frame, the array's scope exit holds",
        [=](Kit& k)
        {
            touch_at(k, 4, "1", "2", false);
        });
    lcv("rngc_empty_count", "touch(&a[0], 0): an empty range holds only the liveness fraction: the call is well formed",
        [=](Kit& k)
        {
            touch_at(k, 4, "0", "0", false);
        });
    lc("rngc_too_long",
       "touch(&a[0], 5) on a four-element array: the shape row (the range is longer than its object) fails at the call, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", touch_at(k, 4, "0", "5", false)->loc, 8U, 0U);
       });
    lc("rngc_negative_count", "touch(&a[0], -1): the shape row (n < 0) fails at the call, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", touch_at(k, 4, "0", "-1", false)->loc, 8U, 0U);
       });
    lc("rngc_null_zero", "touch(NULL, 0) to an unguarded atom: the shape row (p == null) fails, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", touch_at(k, 4, "0", "0", true)->loc, 8U, 0U);
       });
    lc("rngc_null_positive", "touch(NULL, 1): the shape row fails first and ends the path, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", touch_at(k, 4, "0", "1", true)->loc, 8U, 0U);
       });
    lc("rngc_unwritten",
       "touch(&a[0], 4) with a[3] never written: the flag row (uninitialized) fails at the call, one error",
       [=](Kit& k)
       {
           k.fail("uninitialized", touch_at(k, 3, "0", "4", false)->loc);
       });
    // sub-range: what is outside the range keeps its value, what is inside may not
    const auto sub_values = [=](Kit& k, bool inside) -> const Stmt*
    {
        Function* tf {mk_touch(k)};
        const Stmt* c {nullptr};
        caller(k, 4,
               [&](Kit& kk, Function*, Var* a)
               {
                   const Expr* call {nullptr};
                   const Stmt* cs {call_touch(kk, tf, a, "1", "2", &call)};
                   c = kk.chk(inside ? kk.eq_c(ard(kk, a, N(kk, "1")), kk.i32, "2")
                                     : kk.land(kk.eq_c(ard(kk, a, N(kk, "0")), kk.i32, "1"),
                                               kk.eq_c(ard(kk, a, N(kk, "3")), kk.i32, "4")));
                   return std::vector<const Stmt*> {cs, c};
               });
        return c;
    };
    lc("rngc_sub_values",
       "touch(&a[1], 2): a[0] == 1 and a[3] == 4 still hold (outside the range, in the caller's frame)",
       [=](Kit& k)
       {
           sub_values(k, false);
       });
    lc("rngc_sub_values_inside",
       "the same checking a[1] == 2, inside the range the callee owned: the value is lost, the check fails",
       [=](Kit& k)
       {
           k.fail("check", sub_values(k, true)->loc);
       });
    // two calls in a row, and a vacuity twin after them
    lc("rngc_twice",
       "touch(&a[0], 4) twice, then check(false): the range comes back whole each time, and the path after is not empty (the check fails)",
       [=](Kit& k)
       {
           Function* tf {mk_touch(k)};
           const Stmt* c {nullptr};
           caller(k, 4,
                  [&](Kit& kk, Function*, Var* a)
                  {
                      const Expr* c1 {nullptr};
                      const Expr* c2 {nullptr};
                      const Stmt* s1 {call_touch(kk, tf, a, "0", "4", &c1)};
                      const Stmt* s2 {call_touch(kk, tf, a, "0", "4", &c2)};
                      c = kk.chk(kk.no());
                      return std::vector<const Stmt*> {s1, s2, c};
                  });
           k.fail("check", c->loc);
       });
    // two disjoint sub-ranges to a two-range callee, the overlapping ones
    const auto two_ranges = [=](Kit& k, const char* off2, const char* len1, bool vac) -> const Expr*
    {
        Function* tw {mk_pair(k, false)};
        const Expr* call {nullptr};
        caller(k, 4,
               [&](Kit& kk, Function*, Var* a)
               {
                   call = kk.call(tw, {aat(kk, a, N(kk, "0")), N(kk, len1), aat(kk, a, N(kk, off2)), N(kk, "2")});
                   std::vector<const Stmt*> r {kk.es(call)};
                   if (vac)
                   {
                       r.push_back(kk.chk(kk.no()));
                   }
                   return r;
               });
        return call;
    };
    lcv("rngc_two_disjoint", "two(&a[0], 2, &a[2], 2): two owned ranges that share no slot",
        [=](Kit& k)
        {
            two_ranges(k, "2", "2", false);
        });
    lc("rngc_two_overlap",
       "two(&a[0], 3, &a[2], 2): the ranges share a[2]: the separation row of two owned ranges fails at the call, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", two_ranges(k, "2", "3", false)->loc, 17U, 4U);
       });
    // reads_range: the same array twice, overlapping sub-ranges, a write afterwards
    const auto reads_pair = [=](Kit& k, const char* off1, const char* len1, const char* off2, const char* len2,
                                int mode) -> const Expr*
    {
        Function* bf {mk_pair(k, true)};
        const Expr* call {nullptr};
        const Stmt* last {nullptr};
        caller(k, mode == 2 ? 3 : 4,
               [&](Kit& kk, Function*, Var* a)
               {
                   call = kk.call(bf, {aat(kk, a, N(kk, off1)), N(kk, len1), aat(kk, a, N(kk, off2)), N(kk, len2)});
                   std::vector<const Stmt*> r {kk.es(call)};
                   if (mode == 1)
                   {
                       // the permission came back: a write is allowed, and the values were kept
                       r.push_back(awr(kk, a, N(kk, "0"), N(kk, "9")));
                       last = kk.chk(kk.eq_c(ard(kk, a, N(kk, "1")), kk.i32, "2"));
                       r.push_back(last);
                   }
                   return r;
               });
        return call;
    };
    lcv("rngc_reads_same",
        "bothr(&a[0], 4, &a[0], 4): the same range read twice, then a[0] = 9 (the permission came back) and a[1] == 2 (nothing was written)",
        [=](Kit& k)
        {
            reads_pair(k, "0", "4", "0", "4", 1);
        });
    lcv("rngc_reads_overlap", "bothr(&a[0], 3, &a[1], 3): overlapping read ranges: no separation is asked of two reads",
        [=](Kit& k)
        {
            reads_pair(k, "0", "3", "1", "3", 0);
        });
    lc("rngc_reads_too_long",
       "bothr(&a[0], 5, &a[0], 4): the first range is longer than its object: the shape row fails, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", reads_pair(k, "0", "5", "0", "4", 0)->loc, 18U, 0U);
       });
    lc("rngc_reads_unwritten",
       "bothr(&a[0], 4, &a[0], 4) with a[3] unwritten: the flag row fails (a reader needs a value): uninitialized, one error",
       [=](Kit& k)
       {
           k.fail_only("uninitialized", reads_pair(k, "0", "4", "0", "4", 2)->loc, 2U, 0U);
       });
    // reads_range with a value check: values kept across a read-only call, the vacuity twin after it
    lcv("rngc_reads_values", "rdr(&a[0], 4): a reads_range callee writes nothing: a[1] == 2 still holds afterwards",
        [=](Kit& k)
        {
            Function* rf {mk_rdr(k)};
            caller(k, 4,
                   [&](Kit& kk, Function*, Var* a)
                   {
                       const Expr* call {nullptr};
                       const Stmt* cs {call_touch(kk, rf, a, "0", "4", &call)};
                       return std::vector<const Stmt*> {cs, kk.chk(kk.eq_c(ard(kk, a, N(kk, "1")), kk.i32, "2"))};
                   });
        });
    lc("rngc_reads_null_zero",
       "rdr(NULL, 0): a null pointer to an unguarded reads_range: the shape row fails, one error",
       [=](Kit& k)
       {
           Function* rf {mk_rdr(k)};
           const Expr* call {nullptr};
           caller(k, 4,
                  [&](Kit& kk, Function*, Var*)
                  {
                      call = kk.call(rf, {kk.nul(PI(kk)), N(kk, "0")});
                      return std::vector<const Stmt*> {kk.es(call)};
                  });
           k.fail_only("permission", call->loc, 9U, 0U);
       });

    // ---- a borrowed range passed on ----
    // void fwd(int *p, int n[, int k]) requires(owns_range(p, n) && <extra>) ensures(owns_range(p, n)) { touch(<p>, <count>); [check(false);] }
    // mode 0: (p, n); 1: (p, n - 1) with n > 0; 2: (&p[k], n - k) with 0 <= k <= n; 3: (p, n + 1) with n < 100
    const auto forward = [=](Kit& k, int mode, bool vac) -> SrcLoc
    {
        Function* tf {mk_touch(k)};
        std::vector<Kit::P> params {{"p", PI(k)}, {"n", k.i32}};
        if (mode == 2)
        {
            params.push_back({"k", k.i32});
        }
        Function* f {k.fn("fwd", k.vd, params)};
        const Expr* extra {nullptr};
        if (mode == 1)
        {
            extra = k.gt(fx(k, f, 1), N(k, "0"));
        }
        else if (mode == 2)
        {
            extra = k.land(k.ge_c(fx(k, f, 2), k.i32, "0"), k.le(fx(k, f, 2), fx(k, f, 1)));
        }
        else if (mode == 3)
        {
            extra = k.lt(fx(k, f, 1), N(k, "100"));
        }
        k.req(f, extra != nullptr ? k.land(RO(k, f, 0, 1, 0), extra) : RO(k, f, 0, 1, 0));
        k.ens(f, RO(k, f, 0, 1, 0));
        const Expr* base {mode == 2 ? k.b.elem_addr(k.pn(f->param_var(0)), k.pv(f, 2), k.at()) : k.pn(f->param_var(0))};
        const Expr* count {mode == 1   ? k.sub(k.pv(f, 1), N(k, "1"))
                           : mode == 2 ? k.sub(k.pv(f, 1), k.pv(f, 2))
                           : mode == 3 ? k.add(k.pv(f, 1), N(k, "1"))
                                       : k.pv(f, 1)};
        const Expr* call {k.call(tf, {base, count})};
        std::vector<const Stmt*> st {k.es(call)};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            st.push_back(c);
        }
        f->body = k.blk(std::move(st));
        k.done(f);
        return c != nullptr ? c->loc : call->loc;
    };
    lc("rngc_fwd_whole", "a function with owns_range(p, n) passes (p, n) on: the whole range goes and comes back",
       [=](Kit& k)
       {
           forward(k, 0, false);
       });
    lc("rngc_fwd_whole_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           k.fail("check", forward(k, 0, true));
       });
    lc("rngc_fwd_shorter", "(p, n - 1) with n > 0: a shorter range, the last element stays in the frame",
       [=](Kit& k)
       {
           forward(k, 1, false);
       });
    lc("rngc_fwd_shorter_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           k.fail("check", forward(k, 1, true));
       });
    lc("rngc_fwd_tail",
       "(&p[k], n - k) with 0 <= k <= n: the tail of the borrowed range (k == n is the empty one-past-the-end range)",
       [=](Kit& k)
       {
           forward(k, 2, false);
       });
    lc("rngc_fwd_tail_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           k.fail("check", forward(k, 2, true));
       });
    lc("rngc_fwd_one_more",
       "(p, n + 1) of the function's own range: the object may end at p + n, or go on without the callee holding it: two errors, the shape row and then the first marked row",
       [=](Kit& k)
       {
           const SrcLoc call {forward(k, 3, false)};
           k.fail_only("permission", call, 8U, 0U);
           k.fail_only("permission", call, 8U, 4U);
       });

    // ---- guards at the call ----
    // void g(int *p, int n) requires(n == 0 || owns_range(p, n)) ensures(n == 0 || owns_range(p, n)) {}
    auto mk_guarded = [=](Kit& k)
    {
        Function* f {k.fn("g", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)));
        k.ens(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    const auto guarded_call = [=](Kit& k, bool null_base, const char* count) -> const Expr*
    {
        Function* gf {mk_guarded(k)};
        const Expr* call {nullptr};
        caller(k, 4,
               [&](Kit& kk, Function*, Var* a)
               {
                   call = kk.call(gf, {null_base ? kk.nul(PI(kk)) : aat(kk, a, N(kk, "0")), N(kk, count)});
                   return std::vector<const Stmt*> {kk.es(call), kk.es(kk.call(gf, {kk.nul(PI(kk)), N(kk, "0")})),
                                                    kk.chk(kk.eq_c(ard(kk, a, N(kk, "3")), kk.i32, "4"))};
               });
        return call;
    };
    lcv("rngc_guard_ok",
        "g(&a[0], 3) and g(NULL, 0) for requires(n == 0 || owns_range(p, n)): the guard holds nothing for the null call, and the frame keeps a[3] == 4",
        [=](Kit& k)
        {
            guarded_call(k, false, "3");
        });
    lc("rngc_guard_null_positive", "g(NULL, 1): the guard is false, the atom must hold: the shape row fails, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", guarded_call(k, true, "1")->loc, 8U, 0U);
       });
    lc("rngc_guard_too_long", "g(&a[0], 5) on a four-element array: the shape row fails, one error",
       [=](Kit& k)
       {
           k.fail_only("permission", guarded_call(k, false, "5")->loc, 8U, 0U);
       });
    // a bodiless g: its contract is trusted, and f__wf checks the partner by path
    auto mk_trusted = [=](Kit& k, bool ens_guard) -> const Expr*
    {
        Function* f {k.fn("g", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)));
        const Expr* ea {ens_guard ? k.lor(k.eq_c(fx(k, f, 1), k.i32, "0"), RO(k, f, 0, 1, 0)) : RO(k, f, 0, 1, 0)};
        k.ens(f, ea);
        k.trusted(f);
        k.done(f);
        Function* c {k.fn("c", k.vd)};
        Var* a {arr(k, c, "a", k.i32, 2U)};
        std::vector<const Stmt*> st {k.decl(a, nullptr),
                                     awr(k, a, N(k, "0"), N(k, "1")),
                                     awr(k, a, N(k, "1"), N(k, "2")),
                                     k.es(k.call(f, {aat(k, a, N(k, "0")), N(k, "0")})),
                                     k.chk(ens_guard ? k.eq_c(ard(k, a, N(k, "0")), k.i32, "1") : k.no()),
                                     k.es(k.call(f, {aat(k, a, N(k, "0")), N(k, "2")}))};
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        c->body = k.blkc(std::move(st));
        k.done(c);
        return ea;
    };
    lcv("rngc_trusted_partner",
        "a bodiless g with n == 0 || owns_range(p, n) on both sides, called with (a, 0) (a[0] == 1 kept) and (a, 2)",
        [=](Kit& k)
        {
            mk_trusted(k, true);
        });
    lc("rngc_trusted_mismatch",
       "a bodiless requires(n == 0 || owns_range(p, n)) ensures(owns_range(p, n)) called with (a, 0): f__wf reports the ensures atom (permission); the call's ensures then hands back a liveness fraction nothing took, so the caller's state is impossible and check(false) verifies vacuously: the one error is the contract's (audit round 1, B1)",
       [=](Kit& k)
       {
           k.fail("permission", mk_trusted(k, false)->loc);
       });

    // ---- the caller's own read amounts (audit round 1, S2) ----
    // void fw(int *p, int n, int *q, int m) requires(reads_range(p, n) && reads_range(q, m)) ensures(the same) { rdr(p, n); }   and with bothr(p, n, q, m)
    const auto own_amounts = [=](Kit& k, bool both, bool vac) -> const Stmt*
    {
        Function* rf {both ? mk_pair(k, true) : mk_rdr(k)};
        Function* f {k.fn("fw", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}, {"m", k.i32}})};
        k.req(f, k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)));
        k.ens(f, k.land(RR(k, f, 0, 1, 0), RR(k, f, 2, 3, 1)));
        const Expr* call {both ? k.call(rf, {k.pn(f->param_var(0)), k.pv(f, 1), k.pn(f->param_var(2)), k.pv(f, 3)})
                               : k.call(rf, {k.pn(f->param_var(0)), k.pv(f, 1)})};
        const Stmt* c {vac ? k.chk(k.no()) : nullptr};
        f->body =
            k.blk(c != nullptr ? std::vector<const Stmt*> {k.es(call), c} : std::vector<const Stmt*> {k.es(call)});
        k.done(f);
        return c;
    };
    lc("rngc_own_amounts_one",
       "two reads_range parameters that may alias, one passed on: the amount is lowered by the caller's own read amounts, so the first slot's extra fraction is not asked of the others",
       [=](Kit& k)
       {
           own_amounts(k, false, false);
       });
    lc("rngc_own_amounts_one_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           k.fail("check", own_amounts(k, false, true)->loc);
       });
    lc("rngc_own_amounts_both", "both passed on to a two-range reader: verified",
       [=](Kit& k)
       {
           own_amounts(k, true, false);
       });
    lc("rngc_own_amounts_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           k.fail("check", own_amounts(k, true, true)->loc);
       });

    // ---- S44 (d): a range beside a cell ----
    // void mix_o(int *p, int n, int *q) requires(owns_range(p, n) && owns(*q)) ensures(owns_range(p, n) && owns(*q)) {}
    auto mk_mix = [=](Kit& k, bool reads)
    {
        Function* f {k.fn(reads ? "mix_r" : "mix_o", k.vd, {{"p", PI(k)}, {"n", k.i32}, {"q", PI(k)}})};
        const AtomKind cell {reads ? AtomKind::Reads : AtomKind::Owns};
        auto rng = [&](int amount)
        {
            return reads ? RR(k, f, 0, 1, amount) : RO(k, f, 0, 1, amount);
        };
        k.req(f, k.land(k.land(rng(0), k.b.atom(cell, fx(k, f, 2), 1, k.at())), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(rng(0), k.b.atom(cell, fx(k, f, 2), 1, k.at())));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    const auto mix_call = [=](Kit& k, bool reads, bool inside) -> const Expr*
    {
        Function* mf {mk_mix(k, reads)};
        const Expr* call {nullptr};
        caller(k, 4,
               [&](Kit& kk, Function* f, Var* a)
               {
                   Var* x {kk.cell(f, "x", kk.i32)};
                   call = kk.call(mf, {aat(kk, a, N(kk, "0")), N(kk, reads ? "4" : "3"),
                                       inside ? aat(kk, a, N(kk, "1")) : kk.addr(x)});
                   return std::vector<const Stmt*> {kk.decl(x, N(kk, "0")), kk.es(call)};
               });
        return call;
    };
    lcv("rngc_cell_beside_owned", "mix_o(&a[0], 3, &x) with x a separate local: a range and a cell, disjoint",
        [=](Kit& k)
        {
            mix_call(k, false, false);
        });
    lc("rngc_cell_inside_owned",
       "mix_o(&a[0], 3, &a[1]): the owned cell lies inside the owned range: the separation row of a range and a cell fails alone",
       [=](Kit& k)
       {
           k.fail_only("permission", mix_call(k, false, true)->loc, 19U, 9U);
       });
    lcv("rngc_cell_inside_reads", "mix_r(&a[0], 4, &a[1]): a read cell inside a read range is no conflict",
        [=](Kit& k)
        {
            mix_call(k, true, true);
        });

    // ---- item 10 (S44 (i)): an access beside a call whose callee owns a range, and two such calls, in one full expression ----
    // int sumo(int *p, int n) requires(owns_range(p, n) && n == 2) ensures(owns_range(p, n) && result == 7) { return 7; }
    auto mk_sumo = [=](Kit& k)
    {
        Function* f {k.fn("sumo", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0, 1, 0), k.eq_c(fx(k, f, 1), k.i32, "2")));
        k.ens(f, k.land(RO(k, f, 0, 1, 0), k.eq(k.R(f), N(k, "7"))));
        f->body = k.blk({k.ret(N(k, "7"))});
        k.done(f);
        return f;
    };
    // `x = a[<i>] + sumo(&a[1], 2)`: the access and the call in one expression
    const auto place_one = [=](Kit& k, const char* index, const Expr** at_read) -> void
    {
        Function* sf {mk_sumo(k)};
        caller(k, 4,
               [&](Kit& kk, Function* f, Var* a)
               {
                   Var* x {kk.local(f, "x", kk.i32)};
                   const Expr* r {ard(kk, a, N(kk, index))};
                   *at_read = r;
                   return std::vector<const Stmt*> {
                       kk.decl(x, kk.add(r, kk.call(sf, {aat(kk, a, N(kk, "1")), N(kk, "2")}))),
                       kk.chk(kk.eq_c(kk.v(x), kk.i32, std::string(index) == "0" ? "8" : "9"))};
               });
    };
    lcv("rngc_place_read_beside",
        "x = a[0] + sumo(&a[1], 2): the access is outside the callee's range: the placement row holds, x == 8",
        [=](Kit& k)
        {
            const Expr* r {nullptr};
            place_one(k, "0", &r);
        });
    lc("rngc_place_inside",
       "x = a[1] + sumo(&a[1], 2): a[1] lies inside the range the callee owns and C does not order the two: unsequenced at the access, one error",
       [=](Kit& k)
       {
           const Expr* r {nullptr};
           place_one(k, "1", &r);
           k.fail("unsequenced", r->loc);
       });
    const auto two_calls = [=](Kit& k, const char* off2, const Expr** later) -> void
    {
        Function* sf {mk_sumo(k)};
        caller(k, 4,
               [&](Kit& kk, Function* f, Var* a)
               {
                   Var* x {kk.local(f, "x", kk.i32)};
                   const Expr* c1 {kk.call(sf, {aat(kk, a, N(kk, "0")), N(kk, "2")})};
                   const Expr* c2 {kk.call(sf, {aat(kk, a, N(kk, off2)), N(kk, "2")})};
                   *later = c2;
                   return std::vector<const Stmt*> {kk.decl(x, kk.add(c1, c2)), kk.chk(kk.eq_c(kk.v(x), kk.i32, "14"))};
               });
    };
    lcv("rngc_place_two_calls",
        "x = sumo(&a[0], 2) + sumo(&a[2], 2): two calls on disjoint ranges: the pair row holds, x == 14",
        [=](Kit& k)
        {
            const Expr* c {nullptr};
            two_calls(k, "2", &c);
        });
    lc("rngc_place_overlap",
       "x = sumo(&a[0], 2) + sumo(&a[1], 2): the ranges share a[1]: unsequenced at the later call, one error",
       [=](Kit& k)
       {
           const Expr* c {nullptr};
           two_calls(k, "1", &c);
           k.fail("unsequenced", c->loc);
       });

    // ---- a loop in a function without a range contract that calls a range callee (item 10: the callee's owns_range reaches its field) ----
    // void c() { int a[4]; <written>; int i = 0; while (i < 3) decreases(3 - i) { zero_first(&a[0], 4); i = i + 1; } [int y = a[1];|check(a[1] == 2);] }
    const auto loop_call = [=](Kit& k, bool check_value) -> const Stmt*
    {
        Function* zf {mk_zero_first(k)};
        const Stmt* last {nullptr};
        caller(k, 4,
               [&](Kit& kk, Function* f, Var* a)
               {
                   Var* i {kk.local(f, "i", kk.i32)};
                   Var* y {kk.local(f, "y", kk.i32)};
                   const Expr* call {nullptr};
                   const Stmt* cs {call_touch(kk, zf, a, "0", "4", &call)};
                   const Stmt* body {kk.blk({cs, kk.asg(i, kk.add(kk.v(i), N(kk, "1")))})};
                   last = check_value ? kk.chk(kk.eq_c(ard(kk, a, N(kk, "1")), kk.i32, "2"))
                                      : kk.decl(y, ard(kk, a, N(kk, "1")));
                   return std::vector<const Stmt*> {
                       kk.decl(i, N(kk, "0")),
                       kk.whl(kk.lt(kk.v(i), N(kk, "3")), {}, {kk.cl(kk.sub(N(kk, "3"), kk.v(i)))}, body), last};
               });
        return last;
    };
    lcv("rngc_loop_calls_range",
        "a loop that calls zero_first(&a[0], 4) on a local array: the array's permissions and flags are carried, so a[1] is readable afterwards",
        [=](Kit& k)
        {
            loop_call(k, false);
        });
    lc("rngc_loop_calls_range_values",
       "the same checking a[1] == 2 after the loop: the callee owns the range, so the loop does not keep the values: the check fails",
       [=](Kit& k)
       {
           k.fail("check", loop_call(k, true)->loc);
       });

    // ---- audit round 3, B1: two read ranges of one call meeting at a slot a callee lowered: the marked rows take the sum of the two amounts (as a return's do) ----
    // void keep_slot(int *x) requires(reads(*x)) ensures(live(x)) {}: the question-238 callee, which keeps a fraction of the slot and gives the liveness back
    auto mk_keep_slot = [=](Kit& k)
    {
        Function* f {k.fn("keep_slot", k.vd, {{"x", PI(k)}})};
        k.req(f, k.b.atom(AtomKind::Reads, fx(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Live, fx(k, f, 0), 1, k.at()));
        f->body = k.blk({});
        k.done(f);
        return f;
    };
    // void f(int *p, int n) requires(reads_range(p, n) && n > 1) { keep_slot(&p[1]); bothr(p, n, p, n); }
    lc("rngc_reads_meet_kept",
       "keep_slot(&p[1]) leaves slot 1 with a fraction and the liveness whole; bothr(p, n, p, n) then takes two amounts of the slot: each atom's own amount would pass, their sum does not: one permission error, the first sum-form marked row, and no internal failure of Silicon's own check (the limit of question 238, two-atom form)",
       [=](Kit& k)
       {
           Function* kf {mk_keep_slot(k)};
           Function* bf {mk_pair(k, true)};
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0, 1, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           const Expr* c1 {k.call(kf, {k.b.elem_addr(k.pn(f->param_var(0)), N(k, "1"), k.at())})};
           const Expr* c2 {k.call(bf, {k.pn(f->param_var(0)), k.pv(f, 1), k.pn(f->param_var(0)), k.pv(f, 1)})};
           f->body = k.blk({k.es(c1), k.es(c2)});
           k.done(f);
           k.fail_only("permission", c2->loc, 18U, 10U);
       });
    // the same on a local array (the auditor's x3: it exited 3; it is also wrong, the kept fraction would be a lifetime error at the scope's exit, which the error at the call ends the path before)
    lc("rngc_reads_meet_kept_local",
       "keep_slot(&a[1]) on a local array, then bothr(&a[0], 4, &a[0], 4): the same one permission error, the first sum-form marked row at the second call (the program is also wrong at the array's scope exit: the kept fraction; the failed row ends the path first)",
       [=](Kit& k)
       {
           Function* kf {mk_keep_slot(k)};
           Function* bf {mk_pair(k, true)};
           const Expr* c2 {nullptr};
           caller(k, 4,
                  [&](Kit& kk, Function*, Var* a)
                  {
                      c2 = kk.call(bf, {aat(kk, a, N(kk, "0")), N(kk, "4"), aat(kk, a, N(kk, "0")), N(kk, "4")});
                      return std::vector<const Stmt*> {kk.es(kk.call(kf, {aat(kk, a, N(kk, "1"))})), kk.es(c2)};
                  });
           k.fail_only("permission", c2->loc, 18U, 10U);
       });

    // the body of an entry function: the statements, an optional `check(false)` (the vacuity twin), `return 0;`
    auto finish_main = [=](Kit& k, Function* m, std::vector<const Stmt*> st)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            st.push_back(v);
            k.vac_loc = v->loc;
        }
        st.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(std::move(st));
        k.done(m);
        k.prog.set_entry(m); // `main` holds every global from program_start (decision 0041 item 4)
    };
    // ---- audit round 3, S1: a range against a global's cell and against a cell reached through a load ----
    // void fg(int *p, int n) requires(<range>(p, n) && <owns|reads>(g)) ensures(the same) {}   int main() { int a[4] = {...}; fg(<&g, 1 | &a[0], 4>); return 0; }
    const auto range_global = [=](Kit& k, bool reads_range, bool reads_cell, bool overlap) -> const Expr*
    {
        Global* g {k.prog.add_global("g", k.i32, k.at(), nullptr)};
        g->init = N(k, "3");
        g->value = "3";
        auto gatom = [&](int amount)
        {
            return k.b.global_atom(reads_cell ? AtomKind::Reads : AtomKind::Owns, k.b.global_addr(g, k.at()), amount,
                                   k.at());
        };
        Function* f {k.fn("fg", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
        k.req(f, k.land(reads_range ? RR(k, f, 0, 1, 0) : RO(k, f, 0, 1, 0), gatom(1)));
        k.ens(f, k.land(reads_range ? RR(k, f, 0, 1, 0) : RO(k, f, 0, 1, 0), gatom(1)));
        f->body = k.blk({});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* a {arr(k, m, "a", k.i32, 4U)};
        const Expr* call {overlap ? k.call(f, {k.b.global_addr(g, k.at()), N(k, "1")})
                                  : k.call(f, {aat(k, a, N(k, "0")), N(k, "4")})};
        finish_main(k, m,
                    {k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                     awr(k, a, N(k, "2"), N(k, "3")), awr(k, a, N(k, "3"), N(k, "4")), k.es(call)});
        return call;
    };
    lcv("rngc_global_beside_owned",
        "fg(&a[0], 4) with owns_range(p, n) && owns(g): the range is a local array, g a global: the separation row against the global's cell holds",
        [=](Kit& k)
        {
            range_global(k, false, false, false);
        });
    lc("rngc_global_inside_owned",
       "fg(&g, 1) with owns_range(p, n) && owns(g): the range is g's own cell: the separation row of a range and a global's cell fails, permission at the call (was an exit 3)",
       [=](Kit& k)
       {
           k.fail_only("permission", range_global(k, false, false, true)->loc, 19U, 9U);
       });
    lc("rngc_global_inside_reads_range",
       "fg(&g, 1) with reads_range(p, n) && owns(g): one owner among the two: the row is asked",
       [=](Kit& k)
       {
           k.fail_only("permission", range_global(k, true, false, true)->loc, 20U, 10U);
       });
    lc("rngc_global_read_inside_owned",
       "fg(&g, 1) with owns_range(p, n) && reads(g): one owner among the two: the row is asked",
       [=](Kit& k)
       {
           k.fail_only("permission", range_global(k, false, true, true)->loc, 21U, 9U);
       });
    lcv("rngc_global_reads_beside",
        "fg(&a[0], 4) with reads_range(p, n) && reads(g): two readers need no row, and a separate array verifies",
        [=](Kit& k)
        {
            range_global(k, true, true, false);
        });
    // void hq(int **pp, int *q, int n) requires(owns(*pp) && owns(**pp) && owns_range(q, n)) ensures(the same) {}
    // int main() { int a[4] = {...}; int x = 0; int *pa = &<a[1] | x>; hq(&pa, &a[0], 4); return 0; }
    const auto range_loaded = [=](Kit& k, bool inside) -> const Expr*
    {
        Function* f {k.fn("hq", k.vd, {{"pp", k.ptr(PI(k))}, {"q", PI(k)}, {"n", k.i32}})};
        auto atoms = [&](bool tie)
        {
            const Expr* outer {k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at())};
            // the tie `*pp == old(*pp)`: the object `**pp` names on return is the one it named on entry
            const Expr* head {tie ? k.land(outer, k.eq(k.deref(fx(k, f, 0)), k.old(k.deref(fx(k, f, 0))))) : outer};
            return k.land(k.land(head, k.b.atom(AtomKind::Owns, k.deref(fx(k, f, 0)), 1, k.at())), RO(k, f, 1, 2, 2));
        };
        k.req(f, atoms(false));
        k.ens(f, atoms(true));
        f->body = k.blk({});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* a {arr(k, m, "a", k.i32, 4U)};
        Var* x {k.cell(m, "x", k.i32)};
        Var* pa {k.cell(m, "pa", PI(k))};
        const Expr* call {k.call(f, {k.addr(pa), aat(k, a, N(k, "0")), N(k, "4")})};
        finish_main(k, m,
                    {k.decl(a, nullptr), awr(k, a, N(k, "0"), N(k, "1")), awr(k, a, N(k, "1"), N(k, "2")),
                     awr(k, a, N(k, "2"), N(k, "3")), awr(k, a, N(k, "3"), N(k, "4")), k.decl(x, N(k, "0")),
                     k.decl(pa, inside ? aat(k, a, N(k, "1")) : k.addr(x)), k.es(call)});
        return call;
    };
    lcv("rngc_loaded_beside",
        "hq(&pa, &a[0], 4) with pa pointing at a separate cell: the cell reached through *pp lies outside the range",
        [=](Kit& k)
        {
            range_loaded(k, false);
        });
    lc("rngc_loaded_inside",
       "the same with pa == &a[1]: the cell **pp lies inside the owned range: the separation row against a cell reached through a load fails, permission at the call (was an exit 3)",
       [=](Kit& k)
       {
           k.fail_only("permission", range_loaded(k, true)->loc, 29U, 16U);
       });
}

} // namespace
