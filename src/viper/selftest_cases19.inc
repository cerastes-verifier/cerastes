// (included by selftest.cpp) Atoms without a `requires` partner (decision 0052 item 2, M4.5d2b1): an `ensures` atom on
// the result, on a formal the `requires` does not own, or on
// a place reached through the result is COVERED (a partnered atom names the same place: it gives nothing, the caller
// learns the equality) or GIVEN (an output amount, a
// ghost `Perm` result of the method numbered after the `requires` amounts; `out_j := cov ? write : (perm(L) - R) / M`
// at each return, the return row `cov || none < out_j`,
// the postcondition `!cov ==> (none < out_j && <the atom at out_j>)`).  The VIR the encoder builds for `ensures(r,
// live(r))` of a `static` local's address, `ensures(r,
// owns(*r))` of a held global's cell, `owns(*r)` beside `owns(p->x)`, two atoms without a partner on one cell, and a
// bodiless callee that gives one.
// The expectations name the category and the VIR node that carries the row (the return statement for the return rows,
// the use of the pointer for `lifetime`); a failing twin
// says how many checks of that category sit at that line and which of them fails.
namespace {

void define_partnerless_atoms()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto glob = [](Kit& k, const char* name, const char* v, const Function* owner = nullptr) -> Global*
    {
        Global* g {k.prog.add_global(name, k.i32, k.at(), owner)};
        g->init = k.num(k.i32, v);
        g->value = v;
        return g;
    };
    auto ga = [](Kit& k, const Global* g)
    {
        return k.b.global_addr(g, k.at());
    };
    auto gatom = [=](Kit& k, AtomKind kind, const Global* g, int amount, bool implicit = false)
    {
        return k.b.global_atom(kind, ga(k, g), amount, k.at(), implicit);
    };
    // the result `r` with its spelling, and an atom on it (the output amount `amount` is numbered after the `requires`
    // amounts)
    auto RS = [](Kit& k, Function* f)
    {
        return k.b.text(k.R(f), "r");
    };
    auto ratom = [=](Kit& k, AtomKind kind, Function* f, int amount)
    {
        return k.b.atom(kind, RS(k, f), amount, k.at());
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };

    // ---- (a) a `static` local's address: `int *where(void) ensures(r, live(r)) { static int s = 5; return &s; }` ----
    // int main() { int *p = where(); int *q = p; return 0; }      the pointer is used (a copy): the covered live(r)
    // gives liveness, nothing else
    const auto where_live = [=](Kit& k, bool partnerless) -> SrcLoc
    {
        Function* f {k.fn("where", PT(k))};
        Global* s {glob(k, "s", "5", f)};
        k.req(f, gatom(k, AtomKind::Owns, s, 0, true));
        k.ens(f, gatom(k, AtomKind::Owns, s, 0, true));
        if (partnerless)
        {
            k.ens(f, ratom(k, AtomKind::Live, f, 1));
        }
        f->body = k.blk({k.ret(ga(k, s))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* p {k.local(m, "p", PT(k))};
        Var* q {k.local(m, "q", PT(k))};
        const Expr* c {k.call(f, {})};
        m->body = k.blk({k.decl(p, c), k.decl(q, k.pn(p)), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c->loc;
    };
    lc("po_live_static",
       "where() returns &s of its static local with ensures live(r): covered by the implicit owns(s) (the liveness cells are one), so main may use the pointer",
       [=](Kit& k)
       {
           where_live(k, true);
       });
    lc("po_live_static_unframed",
       "the same without the ensures: the caller knows only WF(r), so the copy of the pointer is a lifetime error (static_local_return_address_use.c)",
       [=](Kit& k)
       {
           const SrcLoc at {where_live(k, false)};
           k.fail("lifetime", at, 1U);
       });

    // ---- (a') the same with owns(*r): the caller reads and writes through its own atom on s ----
    // int *where() ensures(r, owns(*r)) { static int s = 5; return [&s|&h]; }   int main() { int *p = where(); *p = 7;
    // check(*p == 7); return 0; }
    const auto where_owns = [=](Kit& k, bool unheld) -> SrcLoc
    {
        Function* f {k.fn("where", PT(k))};
        Global* s {glob(k, "s", "5", f)};
        Global* h {glob(k, "h", "6")};
        k.req(f, gatom(k, AtomKind::Owns, s, 0, true));
        k.ens(f, gatom(k, AtomKind::Owns, s, 0, true));
        k.ens(f, ratom(k, AtomKind::Owns, f, 1));
        const Stmt* r {k.ret(ga(k, unheld ? h : s))};
        f->body = k.blk({r});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* p {k.local(m, "p", PT(k))};
        m->body = k.blk({k.decl(p, k.call(f, {})), k.store(k.pn(p), N(k, "7")),
                         k.chk(k.eq_c(k.deref(k.pn(p)), k.i32, "7")), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return r->loc;
    };
    lc("po_owns_static",
       "ensures owns(*r) of &s with the implicit owns(s) returned: covered by place equality, and main writes and reads *p through its own atom on s",
       [=](Kit& k)
       {
           where_owns(k, false);
       });
    lc("po_owns_static_unheld",
       "the function returns &h, which it does not hold: the atom is not covered, and the return row `none < out` fails",
       [=](Kit& k)
       {
           const SrcLoc at {where_owns(k, true)};
           k.fail_only("permission", at, 14U, 0U);
       });

    // ---- (b), (c) a held global's cell given back: `int *take() requires(owns(g)) ensures(r, owns(*r)) { return &g;
    // }` ----
    // int main() { int *p = take(); *p = 5; check(*p == 5); return 0; }
    const auto take = [=](Kit& k, bool holds, bool other) -> SrcLoc
    {
        Global* g {glob(k, "g", "3")};
        Global* h {glob(k, "h", "4")};
        Function* f {k.fn("take", PT(k))};
        if (holds)
        {
            k.req(f, gatom(k, AtomKind::Owns, g, 0));
        }
        k.ens(f, ratom(k, AtomKind::Owns, f, holds ? 1 : 0));
        const Stmt* r {k.ret(ga(k, other ? h : g))};
        f->body = k.blk({r});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* p {k.local(m, "p", PT(k))};
        m->body = k.blk({k.decl(p, k.call(f, {})), k.store(k.pn(p), N(k, "5")),
                         k.chk(k.eq_c(k.deref(k.pn(p)), k.i32, "5")), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return r->loc;
    };
    lc("po_given_global",
       "take() holds g (requires owns(g), no partner in the ensures) and returns &g with ensures owns(*r): the amount is what it holds, and main writes through the result",
       [=](Kit& k)
       {
           take(k, true, false);
       });
    lc("po_given_nothing_held", "the function holds nothing and still promises owns(*r): the return row fails",
       [=](Kit& k)
       {
           const SrcLoc at {take(k, false, false)};
           k.fail_only("permission", at, 7U, 0U);
       });
    lc("po_given_other_global", "take() holds g but returns &h: the amount is the liveness it holds of &h, none",
       [=](Kit& k)
       {
           const SrcLoc at {take(k, true, true)};
           k.fail_only("permission", at, 7U, 0U);
       });

    // ---- (d) struct S { int x; int y; }   int *getx(struct S *p) requires(owns(p->x)) ensures(owns(*r)) { return
    // &p->x; } ----
    // The function keeps nothing of p->x (no partner in the ensures) and gives it as owns(*r).  The twin writes
    // `owns(p->x)` in the ensures too: written atoms never
    // cover, so the pair must be separate objects and the return row refuses r == &p->x.
    // void use(struct S *p) requires(owns(p->x)) { int *q = getx(p); *q = 3; check(*q == 3); }
    const auto getx = [=](Kit& k, bool pair) -> SrcLoc
    {
        StructType* S {k.strukt("S", {{"x", k.i32}, {"y", k.i32}})};
        Function* f {k.fn("getx", PT(k), {{"p", k.ptr(k.st(S))}})};
        k.req(f, k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 0), 0, k.at()));
        if (pair)
        {
            k.ens(f,
                  k.land(k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 0), 0, k.at()), ratom(k, AtomKind::Owns, f, 1)));
        }
        else
        {
            k.ens(f, ratom(k, AtomKind::Owns, f, 1));
        }
        const Stmt* r {k.ret(k.mem(k.pv(f, 0), 0))};
        f->body = k.blk({r});
        k.done(f);
        Function* u {k.fn("use", k.vd, {{"p", k.ptr(k.st(S))}})};
        k.req(u, k.b.atom(AtomKind::Owns, k.mem(FP(k, u, 0), 0), 0, k.at()));
        Var* q {k.local(u, "q", PT(k))};
        u->body = k.blk({k.decl(q, k.call(f, {k.pv(u, 0)})), k.store(k.pn(q), N(k, "3")),
                         k.chk(k.eq_c(k.deref(k.pn(q)), k.i32, "3"))});
        k.done(u);
        return r->loc;
    };
    lc("po_member_given",
       "owns(*r) given what the function holds of p->x (it keeps nothing of it: no partner in the ensures), r == &p->x: the amount is the structure's liveness it holds; main writes through q",
       [=](Kit& k)
       {
           getx(k, false);
       });
    lc("po_member_pair",
       "owns(p->x) and owns(*r) both written, r == &p->x: a written atom never covers, so the separation row of the pair fails at the return",
       [=](Kit& k)
       {
           const SrcLoc at {getx(k, true)};
           k.fail_only("permission", at, 14U, 0U);
       });

    // ---- a guard: `int *maybe(int c) requires(owns(g)) ensures(r, r == NULL || owns(*r)) { if (c) return NULL; return
    // &g; }` (audit B1) ----
    // On the null path the atom is not given: `out = write`, no row; elsewhere it is given what the function holds.
    const auto maybe = [=](Kit& k, bool other) -> SrcLoc
    {
        Global* g {glob(k, "g", "3")};
        Global* h {glob(k, "h", "4")};
        Function* f {k.fn("maybe", PT(k), {{"c", k.i32}})};
        k.req(f, gatom(k, AtomKind::Owns, g, 0));
        k.ens(f, k.lor(k.eq(RS(k, f), k.nul(PT(k))), ratom(k, AtomKind::Owns, f, 1)));
        const Stmt* r1 {k.ret(k.nul(PT(k)))};
        const Stmt* r2 {k.ret(ga(k, other ? h : g))};
        f->body = k.blk({k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({r1})), r2});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* p {k.local(m, "p", PT(k))};
        m->body = k.blk({k.decl(p, k.call(f, {N(k, "0")})),
                         k.iff(k.ne(k.pn(p), k.nul(PT(k))), k.blk({k.store(k.pn(p), N(k, "5"))})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return r2->loc;
    };
    lc("po_guarded_given",
       "r == NULL || owns(*r): the null return gives nothing and has no row, the other is given what the function holds of g, and main writes through a non-null result",
       [=](Kit& k)
       {
           maybe(k, false);
       });
    lc("po_guarded_unheld",
       "the non-null return is &h, which the function does not hold: its row fails, the null return's does not",
       [=](Kit& k)
       {
           const SrcLoc at {maybe(k, true)};
           k.fail_only("permission", at, 7U, 0U);
       });

    // ---- reads given: `int *peek() requires(owns(g)) ensures(r, reads(*r)) { return &g; }` ----
    lc("po_reads_given",
       "reads(*r) is given the least quotient over its three cells (the liveness cell has only what the callee was given), and the caller reads through it",
       [=](Kit& k)
       {
           Global* g {glob(k, "g", "3")};
           Function* f {k.fn("peek", PT(k))};
           k.req(f, gatom(k, AtomKind::Owns, g, 0));
           k.ens(f, ratom(k, AtomKind::Reads, f, 1));
           f->body = k.blk({k.ret(ga(k, g))});
           k.done(f);
           Function* m {k.fn("main", k.i32)};
           Var* p {k.local(m, "p", PT(k))};
           Var* v {k.local(m, "v", k.i32)};
           m->body = k.blk({k.decl(p, k.call(f, {})), k.decl(v, k.deref(k.pn(p))), k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });

    // ---- two atoms without a partner share what the function holds ----
    // int *both() requires(owns(g)) ensures(r, owns(*r) && live(r)) { return &g; }   int main() { int *p = both(); *p =
    // 1; int *q = p; return 0; }
    lc("po_two_atoms",
       "owns(*r) and live(r), neither covered, split the liveness cell the function holds in halves (M = 2), and the caller gets both",
       [=](Kit& k)
       {
           Global* g {glob(k, "g", "3")};
           Function* f {k.fn("both", PT(k))};
           k.req(f, gatom(k, AtomKind::Owns, g, 0));
           k.ens(f, k.land(ratom(k, AtomKind::Owns, f, 1), ratom(k, AtomKind::Live, f, 2)));
           f->body = k.blk({k.ret(ga(k, g))});
           k.done(f);
           Function* m {k.fn("main", k.i32)};
           Var* p {k.local(m, "p", PT(k))};
           Var* q {k.local(m, "q", PT(k))};
           m->body =
               k.blk({k.decl(p, k.call(f, {})), k.store(k.pn(p), N(k, "1")), k.decl(q, k.pn(p)), k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });

    // ---- a formal's pointee given: void move(int *p, int *q) requires(owns(*q) && p == q) ensures(owns(*p)) {} ----
    // int main() { int x; x = 1; move(&x, &x); return 0; }  (x is no longer held: only through the output of the call)
    const auto move = [=](Kit& k, bool alias) -> SrcLoc
    {
        Function* f {k.fn("move", k.vd, {{"p", PT(k)}, {"q", PT(k)}})};
        if (alias)
        {
            k.req(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 1), 0, k.at()), k.eq(FP(k, f, 0), FP(k, f, 1))));
        }
        k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), alias ? 1 : 0, k.at()));
        f->body = k.blkc({});
        k.done(f);
        return f->end_loc;
    };
    lc("po_formal_given",
       "owns(*p) in the ensures with no atom on p in the requires, but p == q and the function owns *q: the amount is what it holds",
       [=](Kit& k)
       {
           move(k, true);
       });
    lc("po_formal_unheld",
       "the same with nothing in the requires: the function holds nothing of *p, the return row fails at the closing brace",
       [=](Kit& k)
       {
           const SrcLoc at {move(k, false)};
           k.fail_only("permission", at, 7U, 0U);
       });

    // ---- places reached through the result (derived keys): `owns(**r)` is never covered, it is given what the
    // function holds of the place ----
    // int **same(int **pp) requires(owns(*pp) && owns(**pp)) ensures(r, owns(*r) && owns(**r)) { return pp; }
    // The function keeps nothing of `*pp` and `**pp` (no partners in the ensures) and gives both, each the remainder of
    // its own liveness cell.
    const auto same = [=](Kit& k, bool nothing) -> SrcLoc
    {
        Type pp {k.ptr(k.ptr(k.i32))};
        Function* f {k.fn("same", pp, {{"pp", pp}})};
        if (!nothing)
        {
            k.req(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()),
                            k.b.atom(AtomKind::Owns, k.deref(FP(k, f, 0)), 1, k.at())));
        }
        const int nreq {nothing ? 0 : 2};
        k.ens(f,
              k.land(ratom(k, AtomKind::Owns, f, nreq), k.b.atom(AtomKind::Owns, k.deref(RS(k, f)), nreq + 1, k.at())));
        const Stmt* r {k.ret(nothing ? k.nul(pp) : k.pv(f, 0))};
        f->body = k.blk({r});
        k.done(f);
        return r->loc;
    };
    lc("po_result_load",
       "owns(*r) and owns(**r) given what the function holds of *pp and **pp: two liveness cells, each shared by one atom",
       [=](Kit& k)
       {
           same(k, false);
       });
    lc("po_result_load_other",
       "the function holds nothing and returns NULL: owns(*r) has nothing to give and its row fails",
       [=](Kit& k)
       {
           const SrcLoc at {same(k, true)};
           k.fail_only("permission", at, 15U, 0U);
       });

    // ---- a pointer leaf of a struct result: `struct P { int *q; };  struct P mk() requires(owns(g))
    // ensures(owns(*r.q)) { return (struct P){&g}; }` ----
    // int main() { *mk().q = 2; return 0; }     (main gives g away and receives *r.q: the output amount of the leaf
    // place)
    const auto mk_leaf = [=](Kit& k, bool holds) -> SrcLoc
    {
        StructType* P {k.strukt("P", {{"q", PT(k)}})};
        Global* g {glob(k, "g", "1")};
        Function* f {k.fn("mk", k.st(P))};
        if (holds)
        {
            k.req(f, gatom(k, AtomKind::Owns, g, 0));
        }
        k.ens(f, k.b.atom(AtomKind::Owns, k.b.field(k.R(f), 0, k.at()), holds ? 1 : 0, k.at()));
        const Stmt* r {k.ret(k.b.aggregate(k.st(P), {ga(k, g)}, k.at()))};
        f->body = k.blk({r});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.store(k.b.field(k.call(f, {}), 0, k.at()), N(k, "2")), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return r->loc;
    };
    lc("po_result_leaf",
       "owns(*r.q) of a struct result's pointer leaf is given what the function holds of g, and the caller writes through the leaf",
       [=](Kit& k)
       {
           mk_leaf(k, true);
       });
    lc("po_result_leaf_unheld", "the function holds nothing: nothing to give, and the row fails",
       [=](Kit& k)
       {
           const SrcLoc at {mk_leaf(k, false)};
           k.fail_only("permission", at, 7U, 0U);
       });

    // ---- a bodiless callee (trusted) gives an amount: `int *fresh(void) ensures(r, owns(*r));` ----
    lc("po_bodiless_given",
       "a trusted callee promises owns(*r) from nothing: the caller receives the output amount and writes through the result",
       [=](Kit& k)
       {
           Function* f {k.fn("fresh", PT(k))};
           k.ens(f, ratom(k, AtomKind::Owns, f, 0));
           k.trusted(f);
           k.done(f);
           Function* m {k.fn("main", k.i32)};
           Var* p {k.local(m, "p", PT(k))};
           m->body = k.blk({k.decl(p, k.call(f, {})), k.store(k.pn(p), N(k, "2")),
                            k.chk(k.eq_c(k.deref(k.pn(p)), k.i32, "2")), k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    // ---- rejections (check_program): the backstops of the encoder's own ----
    rc("po_rej_nested_pair", "one inside the other",
       [=](Kit& k)
       {
           StructType* S {k.strukt("S", {{"x", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
           k.req(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
           k.ens(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()),
                           k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 0), 1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("po_rej_bodiless_formal", "without a body",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
           k.trusted(f);
           k.done(f);
       });
    rc("po_rej_implicit_after", "the cover must come first",
       [=](Kit& k)
       {
           Function* f {k.fn("where", PT(k))};
           Global* s {glob(k, "s", "5", f)};
           k.req(f, gatom(k, AtomKind::Owns, s, 0, true));
           k.ens(f, ratom(k, AtomKind::Live, f, 1));
           k.ens(f, gatom(k, AtomKind::Owns, s, 0, true));
           f->body = k.blk({k.ret(ga(k, s))});
           k.done(f);
       });
    rc("po_rej_output_number", "not the next one",
       [=](Kit& k)
       {
           Function* f {k.fn("f", PT(k))};
           k.ens(f, ratom(k, AtomKind::Live, f, 3));
           f->body = k.blk({k.ret(k.nul(PT(k)))});
           k.done(f);
       });
}

} // namespace
