// (included by selftest.cpp) Ranges in contracts, M4.5e1b Part B (record 0060 items 3, 4, 5, 7 and 9): `owns_range(p, n)` and `reads_range(p, n)` in a function's `requires`
// and `ensures` (the pointer a parameter, the count an integer expression that reads no memory), the element reads `p[e]` of a pointer parameter in a contract, the
// callee's side of them.  A range atom lowers to one quantifier per fact over the absolute slot (`forall k :: { loc(block_of(p), k) } offset_of(p) <= k < offset_of(p) + n
// ==> ...`), a guard outside each, one conjunct per `requires` and `ensures` line; its `f__wf` is the pure line; a read `p[e]` is framed by a range atom's interval (or a cell
// atom's cell) and has one ghost value per base, index and state; at a return the marked rows (value then flag, the sum form where two read atoms may overlap) stand inside
// `if (g) { }`, then the shape row, the flags, the type's range and the liveness sum.
// A loop in a function with a range atom (Part D) is still refused: the last cases are the backstops.  The callers of range functions are `define_range_calls` (Part C).
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
    rh("range_hold_loop", "before Part D of record 0060",
       [=](Kit& k)
       {
           // void f(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) { int i = 0; while (i < n) { p[i] = 0; i = i + 1; } }
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0, 1, 0));
           k.ens(f, RO(k, f, 0, 1, 0));
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blk({bwr(k, f, 0, k.v(i), N(k, "0")), k.asg(i, k.add(k.v(i), N(k, "1")))})};
           f->body = k.blk(
               {k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), k.pv(f, 1)), {}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
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
        return k.b.text(k.num(k.i32, v), v); // with its spelling, so that a call's messages name the range as written
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
