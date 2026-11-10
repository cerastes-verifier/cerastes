// (included by selftest.cpp) Loop limits of owning fields, Part L-1 (decision 0056 items 2, 3 and 4, M4.5d2b2c): the flags a loop that cannot fill a site keeps (item 2),
// the value set of a lone written owner that has a closure (item 3), and the ghost of a struct local's leaf after a call through an alias of `&s` (item 4).
namespace {

void define_loop_limits_l1()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    // struct S { int *q (owning); int n; }
    auto mk_s = [](Kit& k)
    {
        StructType* S {k.strukt("S", {{"q", k.ptr(k.i32)}, {"n", k.i32}})};
        S->members[0U].owning = true;
        return S;
    };
    // void fill(struct S *p) requires(owns(*p) && (p->q == NULL || owns_block(p->q))) ensures(the same) { free(p->q); p->q = malloc(); if (p->q != NULL) { *p->q = 7; } }
    auto mk_fill = [=](Kit& k, StructType* S, Function* m, Function* fr)
    {
        Function* fill {k.fn("fill", k.vd, {{"p", k.ptr(k.st(S))}})};
        const Var* pp {fill->param_var(0)};
        const auto atoms = [&]()
        {
            const Expr* pq {k.b.text(k.leaf(k.F(fill, 0), 0), "p->q")};
            return k.land(
                k.b.atom(AtomKind::Owns, k.b.text(k.F(fill, 0), "p"), 0, k.at()),
                k.lor(k.eq(k.leaf(k.F(fill, 0), 0), k.nul(PI(k))), k.b.atom(AtomKind::OwnsBlock, pq, -1, k.at())));
        };
        k.req(fill, atoms());
        k.ens(fill, atoms());
        Var* t {k.local(fill, "t", PI(k))};
        fill->body = k.blkc(
            {k.decl(t, k.call(m, {})), k.es(k.call(fr, {k.leaf(k.pn(pp), 0)})), k.lstore(k.pn(pp), 0, k.nul(PI(k))),
             k.iff(k.ne(k.pn(t), k.nul(PI(k))), k.blk({k.store(k.pn(t), N(k, "7")), k.lstore(k.pn(pp), 0, k.pn(t))}))});
        k.done(fill);
        return fill;
    };

    // item 4: a loop that gives a struct local to a callee that refills its leaf.
    //   void use(void) { struct S s; s.q = NULL; s.n = 0; [struct S t; t.q = NULL; t.n = 0; s.q = malloc();] [struct S *a = &s | &t;] int i = 0;
    //                    while (i < 3) { fill([a | &s]); i++; } [free(...);] }
    // mode 0: through the alias a = &s, then free(s.q)           mode 1: the same without the free: the leaf's own row at the end of the body
    // mode 2: fill(&s), then the free                            mode 3: s holds a block, the alias is &t, one call, no loop: free(t.q), free(s.q)
    // mode 4: the same without free(s.q): s's leaf is still held (the alias of another structure leaves its ghost alone)
    const auto alias_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* fill {mk_fill(k, S, m, fr)};
        Function* use {k.fn("use", k.vd)};
        Var* s {k.scell(use, "s", S)};
        Var* t {k.scell(use, "t", S)};
        Var* a {k.local(use, "a", k.ptr(k.st(S)))};
        Var* i {k.local(use, "i", k.i32)};
        std::vector<const Stmt*> body {k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))),
                                       k.lstore(k.addr(s), 1, N(k, "0"))};
        if (mode >= 3)
        {
            body.push_back(k.decl(t, nullptr));
            body.push_back(k.lstore(k.addr(t), 0, k.nul(PI(k))));
            body.push_back(k.lstore(k.addr(t), 1, N(k, "0")));
            body.push_back(k.lstore(k.addr(s), 0, k.call(m, {})));
        }
        if (mode != 2)
        {
            body.push_back(k.decl(a, mode >= 3 ? k.addr(t) : k.addr(s)));
        }
        if (mode >= 3)
        {
            // straight-line (a loop over two structures is slow): the guard of the update is the same
            body.push_back(k.es(k.call(fill, {k.pn(a)})));
        }
        else
        {
            body.push_back(k.decl(i, N(k, "0")));
            const Stmt* lb {
                k.blkc({k.es(k.call(fill, {mode == 2 ? k.addr(s) : k.pn(a)})), k.asg(i, k.add(k.v(i), N(k, "1")))})};
            body.push_back(k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))}, lb));
        }
        if (mode >= 3)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.addr(t), 0)})));
        }
        if (mode == 0 || mode == 2 || mode == 3)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.addr(s), 0)})));
        }
        const Stmt* bs {k.blkc(body)};
        use->body = bs;
        k.done(use);
        return bs->end_loc;
    };
    lc("loop_alias_call_freed",
       "a loop that gives the struct local to a callee through a pointer variable (a = &s), then free(s.q): the leaf's ghost follows the value after each call",
       [=](Kit& k)
       {
           alias_loop(k, 0);
       });
    lc("loop_alias_call_leak",
       "the same without the free: the leaf's own row at the end of the body (leak: owning field s.q)",
       [=](Kit& k)
       {
           k.fail("leak", alias_loop(k, 1), 1U, "", "owning field");
       });
    lc("loop_ref_call_freed", "the same loop with fill(&s), then the free",
       [=](Kit& k)
       {
           alias_loop(k, 2);
       });
    lc("loop_alias_other_freed",
       "s holds a block and the alias is &t: the callee refills t, the guarded update leaves s's ghost alone, and both leaves are freed",
       [=](Kit& k)
       {
           alias_loop(k, 3);
       });
    lc("loop_alias_other_leak",
       "the same without free(s.q): s's leaf is still held at the end of the body (leak: owning field s.q), the alias of another structure did not end it",
       [=](Kit& k)
       {
           k.fail("leak", alias_loop(k, 4), 1U, "", "owning field `s.q`");
       });

    // item 4(a), a struct global: struct S g;  void f(void) requires(owns(g) && g.q == NULL) ensures(owns(g) && (g.q == NULL || owns_block(g.q)))
    //   { struct S *pg = &g; fill(pg); [free(g.q);] g.q = NULL; }
    // the callee gave a block to g.q through the alias: the leaf's ghost is true after the call, so the store of NULL drops a held block unless it is freed first
    const auto global_alias = [=](Kit& k, bool free_first) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* fill {mk_fill(k, S, m, fr)};
        Function* f {k.fn("f", k.vd)};
        Global* g {k.prog.add_global("g", k.st(S), k.at())};
        {
            GlobalLeaf l0;
            l0.kind = Global::Init::Null;
            l0.loc = k.at();
            g->leaves.push_back(l0);
            GlobalLeaf l1;
            l1.kind = Global::Init::Value;
            l1.init = N(k, "0");
            l1.value = "0";
            l1.loc = k.at();
            g->leaves.push_back(l1);
        }
        const auto gq = [&]()
        {
            return k.b.text(k.leaf(k.b.global_addr(g, k.at()), 0), "g.q");
        };
        const auto g_closure = [&]()
        {
            return k.lor(k.eq(k.leaf(k.b.global_addr(g, k.at()), 0), k.nul(PI(k))),
                         k.b.atom(AtomKind::OwnsBlock, gq(), -1, k.at()));
        };
        k.req(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
        k.req(f, k.land(g_closure(), k.eq(k.leaf(k.b.global_addr(g, k.at()), 0), k.nul(PI(k)))));
        k.ens(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()));
        k.ens(f, g_closure());
        Var* pg {k.local(f, "pg", k.ptr(k.st(S)))};
        std::vector<const Stmt*> body {k.decl(pg, k.b.global_addr(g, k.at())), k.es(k.call(fill, {k.pn(pg)}))};
        if (free_first)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.b.global_addr(g, k.at()), 0)})));
        }
        const Stmt* st {k.lstore(k.b.global_addr(g, k.at()), 0, k.nul(PI(k)))};
        body.push_back(st);
        f->body = k.blkc(body);
        k.done(f);
        return st->loc;
    };
    lc("global_leaf_alias_call_freed",
       "a struct global's leaf, a callee that refills it through a pointer variable (pg = &g), then free(g.q) and g.q = NULL",
       [=](Kit& k)
       {
           global_alias(k, true);
       });
    lc("global_leaf_alias_call_drop",
       "the same without the free: the callee gave g.q a block through the alias, and the store of NULL drops it (leak: assigning to owning field g.q)",
       [=](Kit& k)
       {
           k.fail("leak", global_alias(k, false), 1U, "", "assigning to owning field");
       });

    // item 2: struct S { int *q (owning); int n; }  void bump(int **pp) requires(owns(*pp)) ensures(owns(*pp) && *pp == old(*pp));
    //   void f(void) { struct S s = { NULL, 0 }; int *z = NULL; [s.q = malloc(); [if (s.q != NULL) { *s.q = 1; }]] int i = 0;
    //                  while (i < 3) { [free(s.q); s.q = malloc();] bump(&z); i++; }  if (s.q != NULL) { int x = *s.q; }  free(s.q); }
    // mode 0: the loop cannot fill s.q (no store of a pointer, a callee that gives no block): the flag the block had at the entry is kept, the read after the loop is initialized
    // mode 1: the block had no value at the entry: the read after the loop is `uninitialized`
    // mode 2: s.q is NULL at the entry and the loop frees and refills it, so it may fill the site: its flag is not kept, the read after the loop is `uninitialized`
    const auto site_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* bump {k.fn("bump", k.vd, {{"pp", k.ptr(PI(k))}})};
        const auto pp = [&]()
        {
            return k.b.text(k.F(bump, 0), "pp");
        };
        k.req(bump, k.b.atom(AtomKind::Owns, pp(), 0, k.at()));
        k.ens(bump, k.land(k.b.atom(AtomKind::Owns, pp(), 0, k.at()), k.eq(k.deref(pp()), k.old(k.deref(pp())))));
        k.trusted(bump);
        k.done(bump);
        Function* f {k.fn("f", k.vd)};
        Var* s {k.scell(f, "s", S)};
        Var* z {k.cell(f, "z", PI(k))};
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        std::vector<const Stmt*> lbody;
        if (mode == 2)
        {
            lbody.push_back(k.es(k.call(fr, {k.leaf(k.addr(s), 0)})));
            lbody.push_back(k.lstore(k.addr(s), 0, k.call(m, {})));
        }
        lbody.push_back(k.es(k.call(bump, {k.addr(z)})));
        lbody.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(lbody))};
        std::vector<const Stmt*> body {k.decl(s, nullptr),
                                       k.lstore(k.addr(s), 0, k.nul(PI(k))),
                                       k.lstore(k.addr(s), 1, N(k, "0")),
                                       k.decl(z, k.nul(PI(k)))};
        if (mode != 2)
        {
            body.push_back(k.lstore(k.addr(s), 0, k.call(m, {})));
        }
        if (mode == 0)
        {
            body.push_back(k.iff(k.ne(k.leaf(k.addr(s), 0), k.nul(PI(k))),
                                 k.blk({k.store(k.leaf(k.addr(s), 0), N(k, "1"))})));
        }
        body.push_back(k.decl(i, N(k, "0")));
        body.push_back(lp);
        const Expr* rd {k.deref(k.leaf(k.addr(s), 0))};
        body.push_back(k.iff(k.ne(k.leaf(k.addr(s), 0), k.nul(PI(k))), k.blk({k.decl(x, rd)})));
        body.push_back(k.es(k.call(fr, {k.leaf(k.addr(s), 0)})));
        f->body = k.blkc(body);
        k.done(f);
        return rd->loc;
    };
    lc("loop_site_cannot_fill",
       "a loop that stores no pointer and whose callee gives no block cannot fill the leaf s.q: the flag its block had at the entry is kept, the read after the loop is initialized",
       [=](Kit& k)
       {
           site_loop(k, 0);
       });
    lc("loop_site_no_value_at_entry",
       "the same where the block had no value at the entry: the loop keeps what it began with, so the read after the loop is uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", site_loop(k, 1));
       });
    lc("loop_site_may_fill",
       "s.q is NULL at the entry and the loop frees and refills it: the loop may fill the site, its flag is not kept, and the read after the loop is uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", site_loop(k, 2));
       });

    // item 3: void f(int n) { struct S *v = calloc(1, sizeof(struct S)); [owning] int i = 0; while (i < n) { if (v != NULL) { free(v->q); free(v); v = NULL; } i++; }
    //                         if (v != NULL) { [free(v->q);] free(v); } }
    // the loop writes the owning local `v` (only with NULL) and carries the closure of v->q as a synthetic site: the value set `v == ev_v || v == NULL` is emitted, so a `v` that
    // entered the loop NULL stays NULL in the loop's view
    const auto maybe_null = [=](Kit& k, bool free_field) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* frs {heap_free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd, {{"n", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", "3"));
        Var* v {k.local(f, "v", k.ptr(k.st(S)))};
        v->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        const Expr* nn {k.ne(k.pn(v), k.nul(k.ptr(k.st(S))))};
        const Stmt* drop {k.iff(nn, k.blk({k.es(k.call(fr, {k.leaf(k.pn(v), 0)})), k.es(k.call(frs, {k.pn(v)})),
                                           k.asg(v, k.nul(k.ptr(k.st(S))))}))};
        const Stmt* bs {k.blkc({drop, k.asg(i, k.add(k.v(i), N(k, "1")))})};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, bs)};
        std::vector<const Stmt*> tail;
        if (free_field)
        {
            tail.push_back(k.es(k.call(fr, {k.leaf(k.pn(v), 0)})));
        }
        tail.push_back(k.es(k.call(frs, {k.pn(v)})));
        f->body = k.blkc({k.decl(v, k.call(ca, {})), k.decl(i, N(k, "0")), lp,
                          k.iff(k.ne(k.pn(v), k.nul(k.ptr(k.st(S)))), k.blkc(tail))});
        k.done(f);
        return f->end_loc;
    };
    lc("loop_maybe_null_owner",
       "a loop that frees v->q and v and sets v = NULL, where v may be NULL at the entry: v stays NULL in the loop's view, and the free after the loop is of a held block",
       [=](Kit& k)
       {
           maybe_null(k, true);
       });
    lc("loop_maybe_null_owner_drop",
       "the same without free(v->q) after the loop: the field's block is still owned when the function returns (leak)",
       [=](Kit& k)
       {
           k.fail("leak", maybe_null(k, false), 1U, "", "may return while it still owns");
       });
}
