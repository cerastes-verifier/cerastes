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
        std::vector<const Stmt*> body {k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))),
                                       k.lstore(k.addr(s), 1, N(k, "0")), k.decl(z, k.nul(PI(k)))};
        if (mode != 2)
        {
            body.push_back(k.lstore(k.addr(s), 0, k.call(m, {})));
        }
        if (mode == 0)
        {
            body.push_back(
                k.iff(k.ne(k.leaf(k.addr(s), 0), k.nul(PI(k))), k.blk({k.store(k.leaf(k.addr(s), 0), N(k, "1"))})));
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

// Loop limits of owning fields, Part L-2 (decision 0056 items 5, 6 and 7, M4.5d2b2c): the leaf of a global that `main` reaches only by type (under the ghost `hs`), the closure of
// an owning `struct S *` global (the site's block cells are places), and a chain of three owning levels.
void define_loop_limits_l2()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };

    // item 6: struct Leaf { int *x (owning); int k; }  static owning struct Leaf *gp;
    // int main(void) { gp = calloc(1, sizeof(struct Leaf)); if (gp == NULL) { return 1; } int i = 0;
    //                  while (i < 3) { [free(gp->x);] gp->x = malloc(sizeof(int)); i++; } return 0; }
    // the loop carries gp's block and the closure of its owning field gp->x: the block's cells are places for the closure's pointer
    const auto gp_loop = [=](Kit& k, bool free_first, bool free_mid = false) -> SrcLoc
    {
        StructType* Leaf {k.strukt("Leaf", {{"x", PI(k)}, {"k", k.i32}})};
        Leaf->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(Leaf), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* frl {heap_free_view(k, k.st(Leaf))};
        Function* f {k.fn("main", k.i32)};
        k.prog.set_entry(f);
        Global* gp {k.prog.add_global("gp", k.ptr(k.st(Leaf)), k.at())};
        gp->init_kind = Global::Init::Null;
        gp->owning = true;
        const auto gv = [&]()
        {
            return k.deref(k.b.global_addr(gp, k.at()));
        };
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        if (free_mid)
        {
            body.push_back(
                k.iff(k.eq(k.v(i), N(k, "1")), k.blk({k.es(k.call(fr, {k.leaf(gv(), 0)})),
                                                      k.lstore(gv(), 0, k.nul(PI(k))), k.es(k.call(frl, {gv()}))})));
        }
        else
        {
            if (free_first)
            {
                body.push_back(k.es(k.call(fr, {k.leaf(gv(), 0)})));
            }
            body.push_back(k.lstore(gv(), 0, k.call(m, {})));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        f->body = k.blkc({k.store(k.b.global_addr(gp, k.at()), k.call(ca, {})),
                          k.iff(k.eq(gv(), k.nul(k.ptr(k.st(Leaf)))), k.blk({k.ret(N(k, "1"))})), k.decl(i, N(k, "0")),
                          lp, k.ret(N(k, "0"))});
        k.done(f);
        return bs->end_loc;
    };
    lc("loop_global_pointer_closure",
       "a loop in main that frees and replaces gp->x of an owning struct pointer global: the closure of gp's owning field is carried beside gp's block",
       [=](Kit& k)
       {
           gp_loop(k, true);
       });
    lc("loop_global_pointer_closure_drop",
       "the same without the free: the loop's back-edge row reports the old block of gp->x (leak at the end of the body)",
       [=](Kit& k)
       {
           k.fail_only("leak", gp_loop(k, false), 1U, 0U);
       });

    lc("loop_global_pointer_free_block",
       "a body that frees gp->x and gp in one iteration: the rows of the sites end the path (permission at the end of the body), never Silicon's own failure at a pre-read",
       [=](Kit& k)
       {
           k.fail_only("permission", gp_loop(k, false, true), 11U, 1U, "own the whole allocation");
       });

    // item 7: struct Leaf { int *x (owning); int k; }  struct Mid { struct Leaf *c (owning); int j; }  struct Top { struct Mid *b (owning); int m; }
    // int f(void) { struct Top a = { NULL, 0 }; a.b = calloc(); if (a.b == NULL) { return 0; } a.b->c = calloc(); if (a.b->c == NULL) { free(a.b); a.b = NULL; return 0; }
    //               int i = 0; while (i < 3) { MODE; i++; } free(a.b->c->x); free(a.b->c); free(a.b); a.b = NULL; }
    // mode 0: free(a.b->x), a.b->c->x = malloc()      mode 1: a.b->c->x = malloc() (the old block is dropped)
    // mode 2: if (i == 1) { free(a.b->c->x); a.b->c->x = NULL; free(a.b->c); }   (the middle block freed in the body: a mapped row, never Silicon's own failure)
    const auto chain3 = [=](Kit& k, int mode, SrcLoc* fend = nullptr) -> SrcLoc
    {
        StructType* Leaf {k.strukt("Leaf", {{"x", PI(k)}, {"k", k.i32}})};
        Leaf->members[0U].owning = true;
        StructType* Mid {k.strukt("Mid", {{"c", k.ptr(k.st(Leaf))}, {"j", k.i32}})};
        Mid->members[0U].owning = true;
        StructType* Top {k.strukt("Top", {{"b", k.ptr(k.st(Mid))}, {"m", k.i32}})};
        Top->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* cam {heap_alloc_view(k, "calloc", k.st(Mid), true)};
        Function* cal {heap_alloc_view(k, "calloc", k.st(Leaf), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* frl {heap_free_view(k, k.st(Leaf))};
        Function* frm {heap_free_view(k, k.st(Mid))};
        Function* f {k.fn("f", k.i32)};
        Var* a {k.scell(f, "a", Top)};
        Var* i {k.local(f, "i", k.i32)};
        const auto ab = [&]()
        {
            return k.leaf(k.addr(a), 0);
        };
        const auto abc = [&]()
        {
            return k.leaf(ab(), 0);
        };
        std::vector<const Stmt*> body;
        if (mode == 0)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(abc(), 0)})));
            body.push_back(k.lstore(abc(), 0, k.call(m, {})));
        }
        else if (mode == 1)
        {
            body.push_back(k.lstore(abc(), 0, k.call(m, {})));
        }
        else
        {
            body.push_back(
                k.iff(k.eq(k.v(i), N(k, "1")), k.blk({k.es(k.call(fr, {k.leaf(abc(), 0)})),
                                                      k.lstore(abc(), 0, k.nul(PI(k))), k.es(k.call(frl, {abc()}))})));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        std::vector<const Stmt*> tail;
        if (mode != 2)
        {
            tail.push_back(k.es(k.call(fr, {k.leaf(abc(), 0)})));
            tail.push_back(k.es(k.call(frl, {abc()})));
            tail.push_back(k.es(k.call(frm, {ab()})));
            tail.push_back(k.lstore(k.addr(a), 0, k.nul(k.ptr(k.st(Mid)))));
        }
        std::vector<const Stmt*> top {k.decl(a, nullptr),
                                      k.lstore(k.addr(a), 0, k.nul(k.ptr(k.st(Mid)))),
                                      k.lstore(k.addr(a), 1, N(k, "0")),
                                      k.lstore(k.addr(a), 0, k.call(cam, {})),
                                      k.iff(k.eq(ab(), k.nul(k.ptr(k.st(Mid)))), k.blk({k.ret(N(k, "0"))})),
                                      k.lstore(ab(), 0, k.call(cal, {})),
                                      k.iff(k.eq(abc(), k.nul(k.ptr(k.st(Leaf)))),
                                            k.blk({k.es(k.call(frm, {ab()})),
                                                   k.lstore(k.addr(a), 0, k.nul(k.ptr(k.st(Mid)))), k.ret(N(k, "0"))})),
                                      k.decl(i, N(k, "0")),
                                      lp};
        for (const Stmt* t : tail)
        {
            top.push_back(t);
        }
        const Stmt* rt {k.ret(N(k, "0"))};
        top.push_back(rt);
        f->body = k.blkc(top);
        k.done(f);
        if (fend != nullptr)
        {
            *fend = rt->loc;
        }
        return bs->end_loc;
    };
    lc("loop_chain3_replace",
       "a loop that frees and replaces a.b->c->x, three owning levels below a struct local: the closure of the leaf a.b is carried to every depth",
       [=](Kit& k)
       {
           chain3(k, 0);
       });
    lc("loop_chain3_drop",
       "the same without the free: the loop's back-edge row reports the old block of the third level (leak at the end of the body)",
       [=](Kit& k)
       {
           k.fail_only("leak", chain3(k, 1), 1U, 0U);
       });

    lc("loop_chain3_free_mid",
       "a body that frees the middle block (a.b->c) in one iteration: the rows of the sites end the path (permission at the end of the body, then the leaf a.b still holding its block), never Silicon's own failure at a pre-read",
       [=](Kit& k)
       {
           SrcLoc fend;
           const SrcLoc lend {chain3(k, 2, &fend)};
           k.fail_only("permission", lend, 17U, 7U, "own the whole allocation");
           k.fail("leak", fend, 1U, "", "still holds its block");
       });

    // item 5: struct S { int *q (owning); int n; }  struct S g;  void bump(int **pp) requires(owns(*pp)) ensures(owns(*pp) && *pp == old(*pp));
    // int main(void) { g.q = NULL; g.n = 0; g.q = malloc(); int *z = NULL; int i = 0; while (i < 3) { bump(&z); i++; } [int *lk = malloc();] return 0; }
    // the loop reaches g.q only by type: main carries it under the ghost `hs` (held and the cell has a permission); main ends holding the block (exempt)
    const auto main_type = [=](Kit& k, bool leak) -> SrcLoc
    {
        StructType* S {k.strukt("S", {{"q", PI(k)}, {"n", k.i32}})};
        S->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* bump {k.fn("bump", k.vd, {{"pp", k.ptr(PI(k))}})};
        const auto pp = [&]()
        {
            return k.b.text(k.F(bump, 0), "pp");
        };
        k.req(bump, k.b.atom(AtomKind::Owns, pp(), 0, k.at()));
        k.ens(bump, k.land(k.b.atom(AtomKind::Owns, pp(), 0, k.at()), k.eq(k.deref(pp()), k.old(k.deref(pp())))));
        k.trusted(bump);
        k.done(bump);
        Function* f {k.fn("main", k.i32)};
        k.prog.set_entry(f);
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
        Var* z {k.cell(f, "z", PI(k))};
        Var* i {k.local(f, "i", k.i32)};
        Var* lk {k.local(f, "lk", PI(k))};
        const Stmt* bs {k.blkc({k.es(k.call(bump, {k.addr(z)})), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        std::vector<const Stmt*> body {k.lstore(k.b.global_addr(g, k.at()), 0, k.nul(PI(k))),
                                       k.lstore(k.b.global_addr(g, k.at()), 1, N(k, "0")),
                                       k.lstore(k.b.global_addr(g, k.at()), 0, k.call(m, {})),
                                       k.decl(z, k.nul(PI(k))),
                                       k.decl(i, N(k, "0")),
                                       lp};
        if (leak)
        {
            body.push_back(k.decl(lk, k.call(m, {})));
        }
        body.push_back(k.ret(N(k, "0")));
        f->body = k.blkc(body);
        k.done(f);
        return f->end_loc;
    };
    lc("loop_main_type_hs",
       "main holds the block of a struct global's owning leaf across a loop that reaches the leaf only by type: carried under hs, and main ends holding it (exempt)",
       [=](Kit& k)
       {
           main_type(k, false);
       });
    lc("loop_main_type_hs_leak",
       "the same with another block leaked after the loop: the exemption names only the leaf's token, so main's exit row fails",
       [=](Kit& k)
       {
           k.fail("leak", main_type(k, true), 1U, "", "may return while it still owns");
       });
}

} // namespace
