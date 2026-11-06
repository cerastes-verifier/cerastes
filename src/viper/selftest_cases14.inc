// (included by selftest.cpp) Struct objects (decision 0043, M4.5c1): a struct local is a block of member cells (item 4), `s.x` and `p->x` are leaf places
// (item 5), the contract atoms `owns(*p)`, `owns(p->x)`, `reads(*p)` over struct places (item 9), overlap by slots rather than by the atoms' pointee type
// (the leaf-overlap trap `f(&s, &s.x)`, the nested one `g(&t, &t.s)`), the loop's leaves as automatic places with one liveness row per root (item 11),
// scope exits, `must_init` over a struct, path-precise sequencing on struct places (item 12), and the backstop rejections of `check_program`.
// PRE-REGISTERED from the record before lowering ran the cases: each expectation names the category and the VIR node whose position the record gives
// (the read for `uninitialized`, the store for `permission` and `null-dereference`, the value `&p->x` for the null check of an address, the call for a
// separation, the closing brace for a postcondition or a scope exit, the invariant clause for its entry), and each rule has a case that verifies and a
// twin that fails for one reason (invariant 2).  Copies and initializers are expanded by the encoder into leaf reads and stores, so the copy cases here
// are the expansion (read every source leaf into a temporary, then write every destination leaf).
namespace {

void define_struct_objects()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto owns = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at());
    };
    auto reads = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Reads, FP(k, f, i), amount, k.at());
    };
    auto owns_m = [=](Kit& k, Function* f, int i, int member, int amount)
    {
        return k.b.atom(AtomKind::Owns, k.mem(FP(k, f, i), member), amount, k.at());
    };
    auto fm = [=](Kit& k, Function* f, int i, int member)
    {
        return k.leaf(FP(k, f, i), member);
    }; // `p->m` in a contract
    auto old_fm = [=](Kit& k, Function* f, int i, int member)
    {
        return k.old(fm(k, f, i, member));
    }; // `old(p->m)`
    auto mark = [](Function* f, int i, InitMarker m)
    {
        f->params[static_cast<size_t>(i)].init = m;
    };
    // struct S { int x; int y; }   struct T { struct S s; int z; }
    auto mk_s = [](Kit& k)
    {
        return k.strukt("S", {{"x", k.i32}, {"y", k.i32}});
    };
    auto mk_t = [](Kit& k, StructType* s)
    {
        return k.strukt("T", {{"s", k.st(s)}, {"z", k.i32}});
    };
    auto PS = [](Kit& k, StructType* s)
    {
        return k.ptr(k.st(s));
    };
    // A struct local with its declaration.
    auto local_s = [](Kit& k, Function* f, const char* name, StructType* s)
    {
        Var* v {k.scell(f, name, s)};
        return std::make_pair(v, k.decl(v, nullptr));
    };

    // ---- items 4 and 5: a struct local, its leaves read and written by name ----
    // int second(void) ensures(r == 4) { struct S s; s.x = 3; [s.y = 4;] return s.y; }   (mode 1: s.y is never written)
    const auto second = [=](Kit& k, int mode) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("second", k.i32)};
        if (mode == 0)
        {
            k.ens(f, k.eq_c(k.R(f), k.i32, "4"));
        }
        auto [s, ds] = local_s(k, f, "s", S);
        const Expr* rd {k.leaf(k.addr(s), 1)};
        std::vector<const Stmt*> body {ds, k.lstore(k.addr(s), 0, N(k, "3"))};
        if (mode == 0)
        {
            body.push_back(k.lstore(k.addr(s), 1, N(k, "4")));
        }
        body.push_back(k.ret(rd));
        f->body = k.blkc(body);
        k.done(f);
        return rd;
    };
    lc("struct_local",
       "struct S s; s.x = 3; s.y = 4; return s.y: the leaves are cells of the local's block, flags set by the stores, scope exit whole",
       [=](Kit& k)
       {
           second(k, 0);
       });
    lc("struct_local_read_first", "s.y is read before it is written: uninitialized at the read",
       [=](Kit& k)
       {
           k.fail("uninitialized", second(k, 1)->loc);
       });

    // ---- item 9: owns(*p) of a struct pointer parameter ----
    // void bump(struct S *p) requires(owns(*p) && p->x < 100) ensures(owns(*p) && p->x == old(p->x) + 1 && p->y == old(p->y) [+ 1]) { p->x = p->x + 1; }
    const auto bump = [=](Kit& k, bool wrong) -> Function*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("bump", k.vd, {{"p", PS(k, S)}})};
        k.req(f,
              k.land(owns(k, f, 0, 0), k.land(k.lt(fm(k, f, 0, 0), N(k, "100")), k.lt(fm(k, f, 0, 1), N(k, "100")))));
        const Expr* y_post {wrong ? k.add(old_fm(k, f, 0, 1), N(k, "1")) : old_fm(k, f, 0, 1)};
        k.ens(f, k.land(owns(k, f, 0, 0), k.land(k.eq(fm(k, f, 0, 0), k.add(old_fm(k, f, 0, 0), N(k, "1"))),
                                                 k.eq(fm(k, f, 0, 1), y_post))));
        const Expr* pv {k.pn(f->param_var(0))};
        f->body = k.blk({k.lstore(pv, 0, k.add(k.leaf(k.pn(f->param_var(0)), 0), N(k, "1")))});
        k.done(f);
        return f;
    };
    lc("struct_owns_param",
       "owns(*p) over a struct S *: every leaf with its flag and range, p->x = p->x + 1, old(p->x) and old(p->y) in the ensures",
       [=](Kit& k)
       {
           bump(k, false);
       });
    lc("struct_owns_param_post", "the ensures also claims p->y grew: postcondition at the closing brace",
       [=](Kit& k)
       {
           Function* f {bump(k, true)};
           k.fail("postcondition", f->end_loc);
       });

    // void put(struct S *p) [no atom | reads(*p) | owns(p->x)] { p->x = 1 | p->y = 5 }
    const auto put = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("put", k.vd, {{"p", PS(k, S)}})};
        if (mode == 0)
        {
            k.req(f, k.ne(FP(k, f, 0), k.nul(PS(k, S))));
        }
        if (mode == 1)
        {
            k.req(f, reads(k, f, 0, 0));
            k.ens(f, reads(k, f, 0, 0));
        }
        if (mode >= 2)
        {
            k.req(f, owns_m(k, f, 0, 0, 0));
            k.ens(f, owns_m(k, f, 0, 0, 0));
        }
        const Stmt* st {k.lstore(k.pn(f->param_var(0)), mode == 3 ? 1 : 0, N(k, mode == 3 ? "5" : "1"))};
        f->body = k.blk({st});
        k.done(f);
        return st;
    };
    lc("struct_no_atom_write", "p->x = 1 with no atom for *p: permission at the store",
       [=](Kit& k)
       {
           k.fail("permission", put(k, 0)->loc);
       });
    lc("struct_reads_write", "p->x = 1 with reads(*p): every leaf only at a fraction, permission at the store",
       [=](Kit& k)
       {
           k.fail("permission", put(k, 1)->loc);
       });
    lc("struct_owns_member_write", "p->x = 1 with owns(p->x): the one leaf is held in full",
       [=](Kit& k)
       {
           put(k, 2);
       });
    lc("struct_owns_member_other", "p->y = 5 with owns(p->x) only: permission at the store",
       [=](Kit& k)
       {
           k.fail("permission", put(k, 3)->loc);
       });

    // ---- item 5: the null check is on the base pointer ----
    // void put(struct S *p) requires(p == NULL || owns(*p)) ensures(same) { [if (p != NULL)] p->x = 1; }
    const auto guarded = [=](Kit& k, bool guard) -> const Stmt*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("put", k.vd, {{"p", PS(k, S)}})};
        const auto atoms = [&]()
        {
            return k.lor(k.eq(FP(k, f, 0), k.nul(PS(k, S))), owns(k, f, 0, 0));
        };
        k.req(f, atoms());
        k.ens(f, atoms());
        const Stmt* st {k.lstore(k.pn(f->param_var(0)), 0, N(k, "1"))};
        f->body = k.blk({guard ? k.iff(k.ne(k.pn(f->param_var(0)), k.nul(PS(k, S))), st) : st});
        k.done(f);
        return st;
    };
    lc("struct_null_guarded", "p->x = 1 under if (p != NULL), p == NULL || owns(*p)",
       [=](Kit& k)
       {
           guarded(k, true);
       });
    lc("struct_null_deref", "p->x = 1 without the test: null-dereference at the store, on the base pointer",
       [=](Kit& k)
       {
           k.fail("null-dereference", guarded(k, false)->loc);
       });

    // ---- item 5: &p->x as a value is arithmetic on p ----
    // void sink(int *q);   void touch(struct S *p) requires(owns(*p) | p == NULL || owns(*p)) ensures(same) { sink(&p->x); }
    const auto touch = [=](Kit& k, bool maybe_null) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* sink {k.fn("sink", k.vd, {{"q", k.ptr(k.i32)}})};
        k.trusted(sink);
        Function* f {k.fn("touch", k.vd, {{"p", PS(k, S)}})};
        const auto atoms = [&]()
        {
            return maybe_null ? k.lor(k.eq(FP(k, f, 0), k.nul(PS(k, S))), owns(k, f, 0, 0)) : owns(k, f, 0, 0);
        };
        k.req(f, atoms());
        k.ens(f, atoms());
        const Expr* ma {k.mem(k.pn(f->param_var(0)), 0)};
        f->body = k.blk({k.es(k.call(sink, {ma}))});
        k.done(f);
        return ma;
    };
    lc("struct_address_value", "sink(&p->x) with owns(*p): &p->x is formed after the null and liveness checks of p",
       [=](Kit& k)
       {
           touch(k, false);
       });
    lc("struct_address_value_null", "the same with p == NULL || owns(*p): null-dereference at the address",
       [=](Kit& k)
       {
           k.fail("null-dereference", touch(k, true)->loc);
       });

    // struct S *p; { struct S s = ...; p = &s; } [sink(&p->x) inside the block | after it]
    const auto dangling = [=](Kit& k, bool after) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* sink {k.fn("sink", k.vd, {{"q", k.ptr(k.i32)}})};
        k.trusted(sink);
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PS(k, S))};
        auto [s, ds] = local_s(k, f, "s", S);
        const Expr* ma {k.mem(k.v(p), 0)};
        const Stmt* call {k.es(k.call(sink, {ma}))};
        const Stmt* inner {after ? k.blkc({ds, k.lstore(k.addr(s), 0, N(k, "1")), k.lstore(k.addr(s), 1, N(k, "2")),
                                           k.asg(p, k.addr(s))})
                                 : k.blkc({ds, k.lstore(k.addr(s), 0, N(k, "1")), k.lstore(k.addr(s), 1, N(k, "2")),
                                           k.asg(p, k.addr(s)), call})};
        f->body = k.blkc({k.decl(p, nullptr), inner, after ? call : k.es(N(k, "0"))});
        k.done(f);
        return ma;
    };
    lc("struct_address_in_scope", "p = &s; sink(&p->x) inside s's block",
       [=](Kit& k)
       {
           dangling(k, false);
       });
    lc("struct_address_dangling", "sink(&p->x) after s's block ended: lifetime at the address",
       [=](Kit& k)
       {
           k.fail("lifetime", dangling(k, true)->loc);
       });

    // ---- item 9, overlap by slots: the leaf-overlap trap ----
    // void f(struct S *s, int *q) requires(owns(*s) && owns(*q)) ensures(same) { }   use: f(&s, &t.x) | f(&s, &s.x)
    const auto trap = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("f", k.vd, {{"s", PS(k, S)}, {"q", k.ptr(k.i32)}})};
        k.req(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
        k.ens(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
        f->body = k.blk({});
        k.done(f);
        Function* u {k.fn("use", k.vd)};
        auto [s, ds] = local_s(k, u, "s", S);
        auto [t, dt] = local_s(k, u, "t", S);
        const Expr* c {k.call(f, {k.addr(s), alias ? k.mem(k.addr(s), 0) : k.mem(k.addr(t), 0)})};
        u->body = k.blkc({ds, dt, k.lstore(k.addr(s), 0, N(k, "5")), k.lstore(k.addr(s), 1, N(k, "6")),
                          k.lstore(k.addr(t), 0, N(k, "7")), k.lstore(k.addr(t), 1, N(k, "8")), k.es(c)});
        k.done(u);
        return c;
    };
    lc("struct_overlap_trap", "f(&s, &t.x) with owns(*s) && owns(*q): different objects, the atoms share no cell",
       [=](Kit& k)
       {
           trap(k, false);
       });
    lc("struct_overlap_trap_alias",
       "f(&s, &s.x): &s.x is a leaf of *s, so the atoms share it whatever their pointee types: permission at the call",
       [=](Kit& k)
       {
           k.fail_only("permission", trap(k, true)->loc, 2U, 0U, "twice");
       });

    // void g(struct T *t, struct S *s) requires(owns(*t) && owns(*s)) ensures(same) { }   use: g(&t, &u.s) | g(&t, &t.s)
    const auto nested_trap = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* S {mk_s(k)};
        StructType* T {mk_t(k, S)};
        Function* g {k.fn("g", k.vd, {{"t", PS(k, T)}, {"s", PS(k, S)}})};
        k.req(g, k.land(owns(k, g, 0, 0), owns(k, g, 1, 1)));
        k.ens(g, k.land(owns(k, g, 0, 0), owns(k, g, 1, 1)));
        g->body = k.blk({});
        k.done(g);
        Function* u {k.fn("use", k.vd)};
        auto [t, dt] = local_s(k, u, "t", T);
        auto [w, dw] = local_s(k, u, "u", T);
        std::vector<const Stmt*> body {dt, dw};
        for (Var* v : {t, w})
        {
            body.push_back(k.lstore(k.mem(k.addr(v), 0), 0, N(k, "1")));
            body.push_back(k.lstore(k.mem(k.addr(v), 0), 1, N(k, "2")));
            body.push_back(k.lstore(k.addr(v), 1, N(k, "3")));
        }
        const Expr* c {k.call(g, {k.addr(t), alias ? k.mem(k.addr(t), 0) : k.mem(k.addr(w), 0)})};
        body.push_back(k.es(c));
        u->body = k.blkc(body);
        k.done(u);
        return c;
    };
    lc("struct_overlap_nested",
       "g(&t, &u.s) with owns(*t) && owns(*s): two different objects, the nested slot of u is not t's",
       [=](Kit& k)
       {
           nested_trap(k, false);
       });
    lc("struct_overlap_nested_alias",
       "g(&t, &t.s): &t.s is the nested slot of *t, so the atoms share its leaves: permission at the call",
       [=](Kit& k)
       {
           k.fail_only("permission", nested_trap(k, true)->loc, 6U, 0U, "twice");
       });

    // reads over a nested struct beside an owns on an int that may be one of its leaves
    const auto mixed = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("mixed", k.vd, {{"s", PS(k, S)}, {"q", k.ptr(k.i32)}})};
        k.req(f, k.land(reads(k, f, 0, 0), owns(k, f, 1, 1)));
        k.ens(f, k.land(reads(k, f, 0, 0), owns(k, f, 1, 1)));
        f->body = k.blk({});
        k.done(f);
        Function* u {k.fn("use", k.vd)};
        auto [s, ds] = local_s(k, u, "s", S);
        auto [t, dt] = local_s(k, u, "t", S);
        const Expr* c {k.call(f, {k.addr(s), alias ? k.mem(k.addr(s), 0) : k.mem(k.addr(t), 0)})};
        u->body = k.blkc({ds, dt, k.lstore(k.addr(s), 0, N(k, "1")), k.lstore(k.addr(s), 1, N(k, "2")),
                          k.lstore(k.addr(t), 0, N(k, "3")), k.lstore(k.addr(t), 1, N(k, "4")), k.es(c)});
        k.done(u);
        return c;
    };
    lc("struct_overlap_mixed", "mixed(&s, &t.x) with reads(*s) && owns(*q): different objects",
       [=](Kit& k)
       {
           mixed(k, false);
       });
    lc("struct_overlap_mixed_alias",
       "mixed(&s, &s.x): a fraction of s.x and the whole of it name one cell: permission at the call",
       [=](Kit& k)
       {
           k.fail_only("permission", mixed(k, true)->loc, 2U, 0U, "twice");
       });

    // ---- item 4: scope exit and a callee that keeps a leaf ----
    // void dispose(struct S *p) requires(owns(p->x)) [ensures(owns(p->x))];   void leak(void) { struct S s; s.x = 1; s.y = 2; dispose(&s); }
    const auto leak = [=](Kit& k, bool keeps) -> Function*
    {
        StructType* S {mk_s(k)};
        Function* d {k.fn("dispose", k.vd, {{"p", PS(k, S)}})};
        k.req(d, owns_m(k, d, 0, 0, 0));
        if (!keeps)
        {
            k.ens(d, owns_m(k, d, 0, 0, 0));
        }
        d->body = k.blk({});
        k.done(d);
        Function* f {k.fn("leak", k.vd)};
        auto [s, ds] = local_s(k, f, "s", S);
        f->body = k.blkc(
            {ds, k.lstore(k.addr(s), 0, N(k, "1")), k.lstore(k.addr(s), 1, N(k, "2")), k.es(k.call(d, {k.addr(s)}))});
        k.done(f);
        return f;
    };
    lc("struct_dispose_returns",
       "dispose(&s) with requires and ensures owns(p->x): the leaf comes back, the scope exit finds s whole",
       [=](Kit& k)
       {
           leak(k, false);
       });
    lc("struct_dispose_scope_exit", "dispose(&s) that keeps s.x: the scope exit fails (lifetime at the closing brace)",
       [=](Kit& k)
       {
           Function* f {leak(k, true)};
           k.fail("lifetime", f->body->end_loc, 1U, "", "`s.x`");
       });

    // ---- item 11: loops over the leaves ----
    // int loop_members(void) ensures(r == 30 | 31) { struct S s = {0, 0}; int i = 0;
    //   while (i < 10) invariant(0 <= i <= 10 && s.x == i && s.y == 2 * i [+ 1]) measure(10 - i) { s.x += 1; s.y += 2; i += 1; } return s.x + s.y; }
    const auto loop_members = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("loop_members", k.i32)};
        k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "31" : "30"));
        auto [s, ds] = local_s(k, f, "s", S);
        Var* i {k.local(f, "i", k.i32)};
        Clause bound {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le_c(k.v(i), k.i32, "10")))};
        Clause xi {k.cl(k.eq(k.leaf(k.addr(s), 0), k.v(i)))};
        Clause yi {k.cl(
            k.eq(k.leaf(k.addr(s), 1), wrong ? k.add(k.mul(N(k, "2"), k.v(i)), N(k, "1")) : k.mul(N(k, "2"), k.v(i))))};
        Clause measure {k.cl(k.sub(N(k, "10"), k.v(i)))};
        const Stmt* body {k.blk({k.lstore(k.addr(s), 0, k.add(k.leaf(k.addr(s), 0), N(k, "1"))),
                                 k.lstore(k.addr(s), 1, k.add(k.leaf(k.addr(s), 1), N(k, "2"))),
                                 k.asg(i, k.add(k.v(i), N(k, "1")))})};
        const Stmt* w {k.whl(k.lt(k.v(i), N(k, "10")), {bound, xi, yi}, {measure}, body)};
        f->body = k.blkc({ds, k.lstore(k.addr(s), 0, N(k, "0")), k.lstore(k.addr(s), 1, N(k, "0")),
                          k.decl(i, N(k, "0")), w, k.ret(k.add(k.leaf(k.addr(s), 0), k.leaf(k.addr(s), 1)))});
        k.done(f);
        return yi.loc;
    };
    lc("struct_loop_members",
       "a loop writing both leaves of a struct local: every leaf is an automatic place, one liveness row for the root block",
       [=](Kit& k)
       {
           loop_members(k, false);
       });
    lc("struct_loop_members_entry",
       "the invariant says s.y == 2 * i + 1: s.y is 0 on entry, invariant-entry at the clause",
       [=](Kit& k)
       {
           k.fail("invariant-entry", loop_members(k, true));
       });

    // void fill(struct S *p, int n) requires(owns(*p) && 0 <= n <= 100) ensures(owns(*p) && p->x == n && p->y == 2 * n [| == n])
    const auto fill = [=](Kit& k, bool wrong) -> Function*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("fill", k.vd, {{"p", PS(k, S)}, {"n", k.i32}})};
        k.req(f, k.land(owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "100")));
        k.ens(f,
              k.land(owns(k, f, 0, 0), k.land(k.eq(fm(k, f, 0, 0), k.F(f, 1)),
                                              k.eq(fm(k, f, 0, 1), wrong ? k.F(f, 1) : k.mul(N(k, "2"), k.F(f, 1))))));
        Var* i {k.local(f, "i", k.i32)};
        const Var* p {f->param_var(0)};
        Clause bound {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1))))};
        Clause xi {k.cl(k.eq(k.leaf(k.pn(p), 0), k.v(i)))};
        Clause yi {k.cl(k.eq(k.leaf(k.pn(p), 1), k.mul(N(k, "2"), k.v(i))))};
        Clause measure {k.cl(k.sub(k.pv(f, 1), k.v(i)))};
        const Stmt* body {
            k.blk({k.lstore(k.pn(p), 0, k.add(k.leaf(k.pn(p), 0), N(k, "1"))),
                   k.lstore(k.pn(p), 1, k.add(k.leaf(k.pn(p), 1), N(k, "2"))), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        f->body = k.blk({k.decl(i, N(k, "0")), k.lstore(k.pn(p), 0, N(k, "0")), k.lstore(k.pn(p), 1, N(k, "0")),
                         k.whl(k.lt(k.v(i), k.pv(f, 1)), {bound, xi, yi}, {measure}, body)});
        k.done(f);
        return f;
    };
    lc("struct_loop_pointer",
       "a loop over the leaves of a held struct place of a parameter: automatic places per leaf, values kept in the invariant",
       [=](Kit& k)
       {
           fill(k, false);
       });
    lc("struct_loop_pointer_post",
       "the ensures says p->y == n, the loop leaves 2 * n: postcondition at the closing brace",
       [=](Kit& k)
       {
           Function* f {fill(k, true)};
           k.fail("postcondition", f->end_loc);
       });

    // void both(struct S *p, struct S *q) requires(owns(*p) && owns(*q)) { while (...) { p->x += 1; q->y += 1; } }   use: both(&s, &t) | both(&s, &s)
    const auto both = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("both", k.vd, {{"p", PS(k, S)}, {"q", PS(k, S)}})};
        k.req(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
        k.ens(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
        Var* i {k.local(f, "i", k.i32)};
        Clause bound {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le_c(k.v(i), k.i32, "3")))};
        Clause measure {k.cl(k.sub(N(k, "3"), k.v(i)))};
        const Stmt* body {k.blk({k.lstore(k.pn(f->param_var(0)), 0, N(k, "1")),
                                 k.lstore(k.pn(f->param_var(1)), 1, N(k, "2")), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "3")), {bound}, {measure}, body)});
        k.done(f);
        Function* u {k.fn("use", k.vd)};
        auto [s, ds] = local_s(k, u, "s", S);
        auto [t, dt] = local_s(k, u, "t", S);
        const Expr* c {k.call(f, {k.addr(s), alias ? k.addr(s) : k.addr(t)})};
        u->body = k.blkc({ds, dt, k.lstore(k.addr(s), 0, N(k, "1")), k.lstore(k.addr(s), 1, N(k, "2")),
                          k.lstore(k.addr(t), 0, N(k, "3")), k.lstore(k.addr(t), 1, N(k, "4")), k.es(c)});
        k.done(u);
        return c;
    };
    lc("struct_loop_roots",
       "a loop over two struct places: four leaf places, two liveness rows (one per root block), none per leaf",
       [=](Kit& k)
       {
           both(k, false);
       });
    lc("struct_loop_roots_alias", "both(&s, &s): the two owns atoms share the object: permission at the call",
       [=](Kit& k)
       {
           k.fail("permission", both(k, true)->loc, 1U, "", "twice");
       });

    // ---- item 7: a copy is every source leaf read into a temporary, then every destination leaf written ----
    // struct S s; s.x = 1; [s.y = 2;] int a = s.x; int b = s.y; struct S t; t.x = a; t.y = b; return t.y;
    const auto copy = [=](Kit& k, bool unwritten) -> const Expr*
    {
        StructType* S {mk_s(k)};
        Function* f {k.fn("copy", k.i32)};
        auto [s, ds] = local_s(k, f, "s", S);
        auto [t, dt] = local_s(k, f, "t", S);
        Var* a {k.local(f, "a", k.i32)};
        Var* b {k.local(f, "b", k.i32)};
        const Expr* ry {k.leaf(k.addr(s), 1)};
        std::vector<const Stmt*> body {ds, dt, k.lstore(k.addr(s), 0, N(k, "1"))};
        if (!unwritten)
        {
            body.push_back(k.lstore(k.addr(s), 1, N(k, "2")));
        }
        body.push_back(k.decl(a, k.leaf(k.addr(s), 0)));
        body.push_back(k.decl(b, ry));
        body.push_back(k.lstore(k.addr(t), 0, k.v(a)));
        body.push_back(k.lstore(k.addr(t), 1, k.v(b)));
        body.push_back(k.ret(k.leaf(k.addr(t), 1)));
        f->body = k.blkc(body);
        k.done(f);
        return ry;
    };
    lc("struct_copy",
       "t = s as the expansion: both source leaves read into temporaries, both destination leaves written, flags set",
       [=](Kit& k)
       {
           copy(k, false);
       });
    lc("struct_copy_uninit", "s.y was never written: the copy's read of it is uninitialized (question 92)",
       [=](Kit& k)
       {
           k.fail("uninitialized", copy(k, true)->loc);
       });

    // ---- item 9: must_init over a struct ----
    // void init(struct S *p [must_init]) requires(owns(*p)) ensures(owns(*p) [&& p->y == 2]) { p->x = 1; [p->y = 2;] }   int use(void) { struct S s; init(&s); return s.x + s.y; }
    const auto must_init = [=](Kit& k, bool both_written) -> Function*
    {
        StructType* S {mk_s(k)};
        Function* init {k.fn("init", k.vd, {{"p", PS(k, S)}})};
        mark(init, 0, InitMarker::MustInit);
        k.req(init, owns(k, init, 0, 0));
        k.ens(init, owns(k, init, 0, 0));
        std::vector<const Stmt*> w {k.lstore(k.pn(init->param_var(0)), 0, N(k, "1"))};
        if (both_written)
        {
            w.push_back(k.lstore(k.pn(init->param_var(0)), 1, N(k, "2")));
        }
        init->body = k.blk(w);
        k.done(init);
        Function* f {k.fn("use", k.i32)};
        auto [s, ds] = local_s(k, f, "s", S);
        f->body = k.blkc({ds, k.es(k.call(init, {k.addr(s)})), k.ret(k.leaf(k.addr(s), 0))});
        k.done(f);
        return init;
    };
    lc("struct_must_init",
       "must_init over a struct pointee: every leaf is uninitialized in the callee and written; the caller reads both after",
       [=](Kit& k)
       {
           must_init(k, true);
       });
    lc("struct_must_init_missing",
       "the callee never writes p->y, which must_init promises: postcondition at the closing brace",
       [=](Kit& k)
       {
           Function* f {must_init(k, false)};
           k.fail("postcondition", f->end_loc, 1U, "", "`p->y`");
       });

    // ---- item 12: sequencing on struct places ----
    // int two(struct S *p, struct S *q [, struct S *r, int c]) requires(owns(*p) && owns(*q) [&& owns(*r)] && p->x bounded) { [struct S *w = c ? p : r;] return p->x + (w->x = 1); }
    const auto seq = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* S {mk_s(k)};
        std::vector<Kit::P> ps {{"p", PS(k, S)}, {"q", PS(k, S)}};
        if (alias)
        {
            ps.push_back({"r", PS(k, S)});
            ps.push_back({"c", k.i32});
        }
        Function* f {k.fn("two", k.i32, ps)};
        const Expr* bounded {k.land(k.gt(fm(k, f, 0, 0), k.neg(N(k, "100"))), k.lt(fm(k, f, 0, 0), N(k, "100")))};
        const auto atoms = [&]()
        {
            return alias ? k.land(owns(k, f, 0, 0), k.land(owns(k, f, 1, 1), owns(k, f, 2, 2)))
                         : k.land(owns(k, f, 0, 0), owns(k, f, 1, 1));
        };
        k.req(f, k.land(atoms(), bounded));
        k.ens(f, atoms());
        const Expr* target {k.pn(f->param_var(1))};
        std::vector<const Stmt*> body;
        if (alias)
        {
            Var* w {k.local(f, "w", PS(k, S))};
            body.push_back(
                k.decl(w, k.cond(k.ne(k.pv(f, 3), N(k, "0")), k.pn(f->param_var(0)), k.pn(f->param_var(2)))));
            target = k.pn(w);
        }
        const Expr* se {k.b.store_expr(k.mem(target, 0), N(k, "1"), k.at())};
        body.push_back(k.ret(k.add(k.leaf(k.pn(f->param_var(0)), 0), se)));
        f->body = k.blk(body);
        k.done(f);
        return se;
    };
    lc("struct_seq_two_pointers",
       "p->x + (q->x = 1) with owns(*p) && owns(*q): the places differ (separate atoms), path-precise",
       [=](Kit& k)
       {
           seq(k, false);
       });
    lc("struct_seq_alias",
       "w = c ? p : r; p->x + (w->x = 1): w may be p, the read and the write may be one object: unsequenced at the store",
       [=](Kit& k)
       {
           k.fail("unsequenced", seq(k, true)->loc);
       });

    // ---- the backstop rejections of check_program (the by-value parameter and result rejections were retired for lowering cases in selftest_cases15.inc, decision 0044 item 18, question 101) ----
    rc("struct_local_not_a_cell", "address",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.local(f, "s", k.st(S))}; // a struct local that is not address-taken
           f->body = k.blk({k.decl(s, nullptr)});
           k.done(f);
       });
    rc("struct_member_out_of_range", "member",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
           f->body = k.blk({k.es(k.mem(k.pn(f->param_var(0)), 7))});
           k.done(f);
       });
    rc("struct_member_of_a_scalar_pointer", "struct",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.i32)}})};
           f->body = k.blk({k.es(k.mem(k.pn(f->param_var(0)), 0))});
           k.done(f);
       });
}

} // namespace
