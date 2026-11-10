// (included by selftest.cpp) Owning leaves of named structures as owners (decision 0055 items 3, 5 and 6, M4.5d2b2b): the leaf of a by-value parameter's copy
// (its ghost true at entry exactly where the `requires` has a block site on the member), the update of a struct local's leaf after a call through `&s` and through
// an alias, the guarded `ensures` atom that gives the leaf back, the closure of a covered output atom (audit S6) and `main`'s exemption of the blocks a static owner's
// owning fields hold (audit S4).
namespace {

void define_owning_leaves()
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
    // `s.q == NULL || owns_block(s.q)`, `s` the by-value formal 0 of `f`
    auto param_site = [](Kit& k, Function* f)
    {
        const Expr* q {k.b.text(k.b.field(k.F(f, 0), 0, k.at()), "s.q")};
        return k.lor(k.eq(k.b.text(k.b.field(k.F(f, 0), 0, k.at()), "s.q"), k.nul(k.ptr(k.i32))),
                     k.b.atom(AtomKind::OwnsBlock, q, -1, k.at()));
    };

    // void c(struct S s) [requires(s.q == NULL || owns_block(s.q))] { [free(s.q);] }
    // mode 0: the site and the free: the callee's copy holds the block from its entry and ends it       mode 1: the site, no free: the lifetime-end row of the copy's leaf
    // mode 2: no site, the free: the copy holds no block (invalid-free)                                  mode 3: no site, no free: an alias, nothing to end
    const auto by_value = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("c", k.vd, {{"s", k.st(S)}})};
        f->mark_address_taken(f->param_var(0));
        if (mode < 2)
        {
            k.req(f, param_site(k, f));
        }
        std::vector<const Stmt*> body;
        const Expr* fc {nullptr};
        if (mode == 0 || mode == 2)
        {
            fc = k.call(fr, {k.leaf(k.addr(f->param_var(0)), 0)});
            body.push_back(k.es(fc));
        }
        f->body = k.blkc(body);
        k.done(f);
        return fc != nullptr ? fc->loc : f->end_loc;
    };
    lc("owning_leaf_param_freed",
       "a by-value parameter whose contract has owns_block(s.q): its copy holds the block from the entry and frees it",
       [=](Kit& k)
       {
           by_value(k, 0);
       });
    lc("owning_leaf_param_leak",
       "the same callee without the free: the copy's leaf still holds its block when the copy's lifetime ends (leak)",
       [=](Kit& k)
       {
           k.fail("leak", by_value(k, 1), 1U, "", "owning field");
       });
    lc("owning_leaf_param_alias_free",
       "no site on s.q: the copy is an alias, and freeing it is not of an allocation the callee holds (invalid-free)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", by_value(k, 2), 4U, 0U);
       });
    lc("owning_leaf_param_alias", "no site on s.q and no free: an alias owns nothing",
       [=](Kit& k)
       {
           by_value(k, 3);
       });

    // void fill(struct S *p) requires(owns(*p) && (p->q == NULL || owns_block(p->q))) ensures(the same)  { free(p->q); p->q = malloc(); if (p->q != NULL) { *p->q = 7; } }
    // `fill` frees and refills; the callers keep a struct local and pass `&s`, or an alias of it.
    //   void use(void) { struct S s; s.q = NULL; s.n = 0; [struct S *a = &s;] fill([&s | a]); [free(s.q);] }
    // mode 0: &s, then the free         mode 1: &s, no free: the leaf's own row, "owning field `s.q`"
    // mode 2: through the alias, then the free   mode 3: through the alias, no free: no ghost row (the framed update leaves `held` false), the end row of the function
    const auto fill_caller = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
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
        Function* use {k.fn("use", k.vd)};
        Var* s {k.scell(use, "s", S)};
        Var* a {k.local(use, "a", k.ptr(k.st(S)))};
        std::vector<const Stmt*> body {k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))),
                                       k.lstore(k.addr(s), 1, N(k, "0"))};
        if (mode >= 2)
        {
            body.push_back(k.decl(a, k.addr(s)));
        }
        body.push_back(k.es(k.call(fill, {mode >= 2 ? k.pn(a) : k.addr(s)})));
        if (mode == 0 || mode == 2)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.addr(s), 0)})));
        }
        const Stmt* bs {k.blkc(body)};
        use->body = bs;
        k.done(use);
        // the lifetime of `s` ends with the body block, an ordinary block's end
        return mode == 3 ? use->end_loc : bs->end_loc;
    };
    lc("owning_leaf_call_ref_freed",
       "&s to a callee that refills the leaf, then the free: the leaf's ghost follows the value after the call",
       [=](Kit& k)
       {
           fill_caller(k, 0);
       });
    lc("owning_leaf_call_ref_leak",
       "the same without the free: the lifetime-end row of the leaf (leak: owning field s.q)",
       [=](Kit& k)
       {
           k.fail("leak", fill_caller(k, 1), 1U, "", "owning field");
       });
    lc("owning_leaf_call_alias_freed",
       "an alias of &s to the callee, then the free of s.q: the framed update keeps the ghost false and the free needs no row",
       [=](Kit& k)
       {
           fill_caller(k, 2);
       });
    lc("owning_leaf_call_alias_leak",
       "the same without the free: the ghost stayed false, so the block is reported by the end row of the function",
       [=](Kit& k)
       {
           k.fail("leak", fill_caller(k, 3), 1U, "", "may return while it still owns");
       });

    // void maybe(struct S *p, int c) requires(owns(*p) && (p->q == NULL || owns_block(p->q))) ensures(c != 0 ? (the same) : 1) { if (c == 0) { free(p->q); } }  (both lists also say p->q == NULL)
    //   void use(int c) { struct S s; s.q = NULL; s.n = 0; maybe(&s, c); }
    // the callee's `ensures` atom is guarded: after the call the cell is read only where the function holds it again (a callee that took it leaves `held_s_q` false, and
    // the failure is the caller's, at the lifetime of `s`, never an unexplained Silicon error)
    lc("owning_leaf_call_guarded_ensures",
       "a callee whose ensures gives the leaf back only when c != 0: where it did not, the cell is gone and the caller's failure is its own (lifetime of s)",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* fr {heap_free_view(k, k.i32)};
           Function* maybe {k.fn("maybe", k.vd, {{"p", k.ptr(k.st(S))}, {"c", k.i32}})};
           // the leaf is NULL on entry and on the `c != 0` return, so the callee gives back nothing the caller's leaf row could report
           const auto atoms = [&]()
           {
               const Expr* pq {k.b.text(k.leaf(k.F(maybe, 0), 0), "p->q")};
               return k.land(k.land(k.b.atom(AtomKind::Owns, k.b.text(k.F(maybe, 0), "p"), 0, k.at()),
                                    k.lor(k.eq(k.leaf(k.F(maybe, 0), 0), k.nul(PI(k))),
                                          k.b.atom(AtomKind::OwnsBlock, pq, -1, k.at()))),
                             k.eq(k.leaf(k.F(maybe, 0), 0), k.nul(PI(k))));
           };
           k.req(maybe, atoms());
           k.ens(maybe, k.cond(k.ne(k.F(maybe, 1), N(k, "0")), atoms(), k.yes()));
           maybe->body = k.blkc({k.iff(k.eq(k.pv(maybe, 1), N(k, "0")),
                                       k.blk({k.es(k.call(fr, {k.leaf(k.pn(maybe->param_var(0)), 0)}))}))});
           k.done(maybe);
           Function* use {k.fn("use", k.vd, {{"c", k.i32}})};
           Var* s {k.scell(use, "s", S)};
           const Stmt* bs {k.blkc({k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))),
                                   k.lstore(k.addr(s), 1, N(k, "0")), k.es(k.call(maybe, {k.addr(s), k.pv(use, 0)}))})};
           use->body = bs;
           k.done(use);
           // the rows of s.q, s.n and the liveness cell: the callee took them on the path where c == 0 (the first one fails first)
           k.fail_only("lifetime", bs->end_loc, 3U, 0U, "may still be in use");
       });

    // audit S6: struct S *get(void) ensures(r, owns(*r)) { static struct S s; [free(s.q);] return &s; }: the implicit owns(s) of `get` covers the output atom, so the
    // closure of `owns(*r)` (r->q == NULL || owns_block(r->q)) is the block the implicit atom's own closure returns: it stands under !cov
    const auto static_cover = [=](Kit& k, bool drop) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("get", k.ptr(k.st(S)))};
        Global* s {k.prog.add_global("s", k.st(S), k.at(), f)};
        {
            GlobalLeaf l0;
            l0.kind = Global::Init::Null;
            l0.loc = k.at();
            s->leaves.push_back(l0);
            GlobalLeaf l1;
            l1.kind = Global::Init::Value;
            l1.init = N(k, "0");
            l1.value = "0";
            l1.loc = k.at();
            s->leaves.push_back(l1);
        }
        const auto gq = [&]()
        {
            return k.b.text(k.leaf(k.b.global_addr(s, k.at()), 0), "s.q");
        };
        const auto g_closure = [&]()
        {
            return k.lor(k.eq(k.leaf(k.b.global_addr(s, k.at()), 0), k.nul(PI(k))),
                         k.b.atom(AtomKind::OwnsBlock, gq(), -1, k.at()));
        };
        k.req(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(s, k.at()), 0, k.at(), true));
        k.req(f, g_closure());
        // the implicit atoms come before the output atom (audit B5)
        k.ens(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(s, k.at()), 0, k.at(), true));
        k.ens(f, g_closure());
        const Expr* rq {k.b.text(k.leaf(k.R(f), 0), "r->q")};
        k.ens(f, k.land(k.b.atom(AtomKind::Owns, k.b.text(k.R(f), "r"), 1, k.at()),
                        k.lor(k.eq(k.leaf(k.R(f), 0), k.nul(PI(k))), k.b.atom(AtomKind::OwnsBlock, rq, -1, k.at()))));
        std::vector<const Stmt*> body;
        if (drop)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.b.global_addr(s, k.at()), 0)})));
        }
        const Stmt* rs {k.ret(k.b.global_addr(s, k.at()))};
        body.push_back(rs);
        f->body = k.blkc(body);
        k.done(f);
        return rs->loc;
    };
    lc("owning_cover_closure_once",
       "return &s of a static struct with an owning field, ensures(r, owns(*r)): the block of s.q is returned once, by the implicit atom",
       [=](Kit& k)
       {
           static_cover(k, false);
       });
    lc("owning_cover_closure_dropped",
       "the same after free(s.q): the implicit atom's closure still fails at the return, the cover hides no other row (permission)",
       [=](Kit& k)
       {
           // the rows of `s.q`'s block (the implicit atom's closure) and of `r->q`'s (the output atom's, under !cov) stand at the return; the first of the former fails
           k.fail_only("permission", static_cover(k, true), 5U, 1U, "`s.q` points to");
       });

    // audit S4: struct Leaf { int *x (owning); int *y; }  struct Mid { struct Leaf *b (owning); int m; }  static struct Mid g;
    // int main(void) { g.b = calloc(1, sizeof(struct Leaf)); if (g.b != NULL) { g.b->[x | y] = malloc(); } return 0; }
    // main ends while the owning leaf g.b holds a structure whose owning field g.b->x holds a block: both blocks are exempt (a chain under a static owner); the plain field
    // g.b->y is not
    const auto main_chain = [=](Kit& k, int field) -> SrcLoc
    {
        StructType* Leaf {k.strukt("Leaf", {{"x", PI(k)}, {"y", PI(k)}})};
        Leaf->members[0U].owning = true;
        StructType* Mid {k.strukt("Mid", {{"b", k.ptr(k.st(Leaf))}, {"m", k.i32}})};
        Mid->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(Leaf), true)};
        Function* f {k.fn("main", k.i32)};
        k.prog.set_entry(f);
        Global* g {k.prog.add_global("g", k.st(Mid), k.at())};
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
        const auto gb = [&]()
        {
            return k.leaf(k.b.global_addr(g, k.at()), 0);
        };
        const Stmt* st {k.lstore(gb(), field, k.call(m, {}))};
        f->body = k.blkc({k.lstore(k.b.global_addr(g, k.at()), 0, k.call(ca, {})),
                          k.iff(k.ne(gb(), k.nul(k.ptr(k.st(Leaf)))), k.blk({st})), k.ret(N(k, "0"))});
        k.done(f);
        return f->end_loc;
    };
    lc("owning_main_chain_exempt",
       "main ends holding g.b and the block of g.b->x: a static owner's owning fields, to any depth, are exempt",
       [=](Kit& k)
       {
           main_chain(k, 0);
       });
    lc("owning_main_chain_plain_leak",
       "the block goes to the plain field g.b->y: not exempt, the leak row of main fails",
       [=](Kit& k)
       {
           k.fail("leak", main_chain(k, 1), 1U, "", "may return while it still owns");
       });
    // int c(struct S s, int k) requires(k != 0 ? (s.q == NULL || owns_block(s.q)) : 1) { if (k != 0) { free(s.q); } return 0; }
    // the site holds the block only where its guard does: the copy's leaf is held at entry under `k != 0`, so a callee that frees on that path and leaves the alias alone on the other
    // has no leaf to end (without the guard `held` is true on the alias path and the lifetime-end row reports it)
    lc("owning_leaf_param_guarded",
       "a by-value parameter's site under a guard: the leaf is held at entry only where the guard holds",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* fr {heap_free_view(k, k.i32)};
           Function* f {k.fn("c", k.i32, {{"s", k.st(S)}, {"k", k.i32}})};
           f->mark_address_taken(f->param_var(0));
           k.req(f, k.cond(k.ne(k.F(f, 1), N(k, "0")), param_site(k, f), k.yes()));
           f->body = k.blkc(
               {k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.blk({k.es(k.call(fr, {k.leaf(k.addr(f->param_var(0)), 0)}))})),
                k.ret(N(k, "0"))});
           k.done(f);
       });

    // static struct S g;  void maybe(int c) requires(owns(g) && g.q == NULL) ensures(c != 0 ? (owns(g) && g.q == NULL) : 1);  int main(void) { maybe(0); return 0; }
    // the callee's `ensures` atom on the struct global is guarded: where its guard is false the callee took the cell, so main reads the leaf after the call only where it holds it
    lc("owning_global_leaf_guarded_ensures",
       "a bodiless callee that gives the global's leaf back only when c != 0: main reads the leaf after the call only where it holds it",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* maybe {k.fn("maybe", k.vd, {{"c", k.i32}})};
           k.trusted(maybe);
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
           const auto atoms = [&]()
           {
               const Expr* gq {k.b.text(k.leaf(k.b.global_addr(g, k.at()), 0), "g.q")};
               return k.land(k.land(k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at()),
                                    k.lor(k.eq(k.leaf(k.b.global_addr(g, k.at()), 0), k.nul(PI(k))),
                                          k.b.atom(AtomKind::OwnsBlock, gq, -1, k.at()))),
                             k.eq(k.leaf(k.b.global_addr(g, k.at()), 0), k.nul(PI(k))));
           };
           k.req(maybe, atoms());
           k.ens(maybe, k.cond(k.ne(k.F(maybe, 0), N(k, "0")), atoms(), k.yes()));
           k.done(maybe);
           Function* f {k.fn("main", k.i32)};
           k.prog.set_entry(f);
           f->body = k.blkc({k.es(k.call(maybe, {N(k, "0")})), k.ret(N(k, "0"))});
           k.done(f);
       });

    // struct Leaf { int *x (owning); int *y; }  static owning struct Leaf *gp;
    // int main(void) { gp = calloc(1, sizeof(struct Leaf)); if (gp != NULL) { gp->[x | y] = malloc(); } return 0; }
    // the owning field gp->x of an owning `struct S *` global is exempt at main's end with the global's own block; the plain field gp->y is not
    const auto main_pointer = [=](Kit& k, int field) -> SrcLoc
    {
        StructType* Leaf {k.strukt("Leaf", {{"x", PI(k)}, {"y", PI(k)}})};
        Leaf->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(Leaf), true)};
        Function* f {k.fn("main", k.i32)};
        k.prog.set_entry(f);
        Global* gp {k.prog.add_global("gp", k.ptr(k.st(Leaf)), k.at())};
        gp->init_kind = Global::Init::Null;
        gp->owning = true;
        const auto gv = [&]()
        {
            return k.deref(k.b.global_addr(gp, k.at()));
        };
        f->body = k.blkc({k.store(k.b.global_addr(gp, k.at()), k.call(ca, {})),
                          k.iff(k.ne(gv(), k.nul(k.ptr(k.st(Leaf)))), k.blk({k.lstore(gv(), field, k.call(m, {}))})),
                          k.ret(N(k, "0"))});
        k.done(f);
        return f->end_loc;
    };
    lc("owning_main_pointer_exempt",
       "main ends holding the owning struct pointer global and the block of its owning field gp->x: both exempt",
       [=](Kit& k)
       {
           main_pointer(k, 0);
       });
    lc("owning_main_pointer_plain_leak",
       "the block goes to the plain field gp->y: not exempt, the leak row of main fails",
       [=](Kit& k)
       {
           k.fail("leak", main_pointer(k, 1), 1U, "", "may return while it still owns");
       });
    // decision 0055 item 7: loops over owning leaves.
    // void f(void) { struct S s; s.q = NULL; s.n = 0; int i = 0; while (i < 3) { [free(s.q);] s.q = malloc(); i++; } free(s.q); }
    // the loop names the leaf of a struct local and has no written invariant on it: a synthetic site on `s.q` carries it (held_s_q == (s.q != NULL) is invariant), the
    // reassignment row stays at the store
    const auto leaf_loop = [=](Kit& k, bool free_first) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* s {k.scell(f, "s", S)};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        if (free_first)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.addr(s), 0)})));
        }
        const Stmt* st {k.lstore(k.addr(s), 0, k.call(m, {}))};
        body.push_back(st);
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(body))};
        f->body = k.blkc({k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))), k.lstore(k.addr(s), 1, N(k, "0")),
                          k.decl(i, N(k, "0")), lp, k.es(k.call(fr, {k.leaf(k.addr(s), 0)}))});
        k.done(f);
        return st->loc;
    };
    lc("owning_leaf_loop_replace",
       "a loop that frees and replaces the owning leaf s.q of a struct local each iteration: carried by a synthetic site",
       [=](Kit& k)
       {
           leaf_loop(k, true);
       });
    lc("owning_leaf_loop_drop",
       "the same without the free: the reassignment row at the store fails (leak: assigning to owning field s.q)",
       [=](Kit& k)
       {
           k.fail_only("leak", leaf_loop(k, false), 1U, 0U, "assigning to owning field");
       });

    // void g(void) { owning struct S *v = calloc(); if (v == NULL) { return; } int i = 0; while (i < 3) { [free(v->q);] v->q = malloc(); i++; } free(v->q); free(v); }
    // (audit S5) the owning local carries the closure of its pointee's owning fields: v == NULL || (v->q == NULL || owns_block(v->q)) is a synthetic site
    const auto ptr_loop = [=](Kit& k, bool free_first) -> SrcLoc
    {
        StructType* S {mk_s(k)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* frs {heap_free_view(k, k.st(S))};
        Function* f {k.fn("g", k.vd)};
        Var* v {k.local(f, "v", k.ptr(k.st(S)))};
        v->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        if (free_first)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.pn(v), 0)})));
        }
        body.push_back(k.lstore(k.pn(v), 0, k.call(m, {})));
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        f->body =
            k.blkc({k.decl(v, k.call(ca, {})), k.iff(k.eq(k.pn(v), k.nul(k.ptr(k.st(S)))), k.blk({k.ret(nullptr)})),
                    k.decl(i, N(k, "0")), lp, k.es(k.call(fr, {k.leaf(k.pn(v), 0)})), k.es(k.call(frs, {k.pn(v)}))});
        k.done(f);
        return bs->end_loc;
    };
    lc("owning_ptr_loop_replace",
       "a loop that frees and replaces v->q of an owning local struct pointer each iteration: the closure of its field is carried",
       [=](Kit& k)
       {
           ptr_loop(k, true);
       });
    lc("owning_ptr_loop_drop",
       "the same without the free: the loop's back-edge row reports the block it drops (leak at the end of the body)",
       [=](Kit& k)
       {
           k.fail_only("leak", ptr_loop(k, false), 1U, 0U);
       });

    // ---- the final re-audit's fixes (M4.5d2b2b) ----

    // (B1) struct S *get(void) ensures(r, r == NULL || owns(*r)); struct S *pick(void) ensures(r, owns(*r)) { static struct S s; struct S *x = get(); if (x == NULL) { return &s; }
    // return x; }: the closure of the output atom is the implicit atom's only where the result is `&s`; the path that returns `x` hands back no block for x->q, and the leak
    // row must say so (the exemption of the closure site is as dead as its return row)
    lc("owning_cover_pick_leak",
       "a function that returns &s on one path and a callee's structure on another, ensures(r, owns(*r)): the callee's field block is neither handed back nor exempt (leak at the closing brace)",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* gt {k.fn("get", k.ptr(k.st(S)))};
           {
               const Expr* gq {k.b.text(k.leaf(k.R(gt), 0), "r->q")};
               k.ens(gt, k.lor(k.eq(k.R(gt), k.nul(k.ptr(k.st(S)))),
                               k.land(k.b.atom(AtomKind::Owns, k.b.text(k.R(gt), "r"), 0, k.at()),
                                      k.lor(k.eq(k.leaf(k.R(gt), 0), k.nul(PI(k))),
                                            k.b.atom(AtomKind::OwnsBlock, gq, -1, k.at())))));
               k.trusted(gt);
               k.done(gt);
           }
           Function* f {k.fn("pick", k.ptr(k.st(S)))};
           Global* s {k.prog.add_global("s", k.st(S), k.at(), f)};
           {
               GlobalLeaf l0;
               l0.kind = Global::Init::Null;
               l0.loc = k.at();
               s->leaves.push_back(l0);
               GlobalLeaf l1;
               l1.kind = Global::Init::Value;
               l1.init = N(k, "0");
               l1.value = "0";
               l1.loc = k.at();
               s->leaves.push_back(l1);
           }
           const auto gq = [&]()
           {
               return k.b.text(k.leaf(k.b.global_addr(s, k.at()), 0), "s.q");
           };
           const auto g_closure = [&]()
           {
               return k.lor(k.eq(k.leaf(k.b.global_addr(s, k.at()), 0), k.nul(PI(k))),
                            k.b.atom(AtomKind::OwnsBlock, gq(), -1, k.at()));
           };
           k.req(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(s, k.at()), 0, k.at(), true));
           k.req(f, g_closure());
           k.ens(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(s, k.at()), 0, k.at(), true));
           k.ens(f, g_closure());
           const Expr* rq {k.b.text(k.leaf(k.R(f), 0), "r->q")};
           k.ens(f,
                 k.land(k.b.atom(AtomKind::Owns, k.b.text(k.R(f), "r"), 1, k.at()),
                        k.lor(k.eq(k.leaf(k.R(f), 0), k.nul(PI(k))), k.b.atom(AtomKind::OwnsBlock, rq, -1, k.at()))));
           Var* x {k.local(f, "x", k.ptr(k.st(S)))};
           f->body = k.blkc({k.decl(x, k.call(gt, {})),
                             k.iff(k.eq(k.pn(x), k.nul(k.ptr(k.st(S)))), k.blk({k.ret(k.b.global_addr(s, k.at()))})),
                             k.ret(k.pn(x))});
           k.done(f);
           k.fail("leak", f->end_loc, 1U, "", "may return while it still owns");
       });

    // (S1) void g(void) { owning struct S *v = calloc(); if (v == NULL) { return; } int i = 0; while (i < 3) { if (i == 2 && v != NULL) { free(v->q); free(v); v = NULL; } i++; } }
    // the loop ASSIGNS the owning local: its closure site is read where `v->q` has a value (the carrier of an assigned owner keeps the cells and not their flags)
    lc("owning_ptr_loop_assigned",
       "a loop that frees v->q and v and sets v = NULL, the owning local assigned in the loop: the closure of v->q is carried where its cell has a value",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
           Function* ca {heap_alloc_view(k, "calloc", k.st(S), true)};
           Function* fr {heap_free_view(k, k.i32)};
           Function* frs {heap_free_view(k, k.st(S))};
           Function* f {k.fn("g", k.vd)};
           Var* v {k.local(f, "v", k.ptr(k.st(S)))};
           v->owning = true;
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* drop {k.iff(k.land(k.eq(k.v(i), N(k, "2")), k.ne(k.pn(v), k.nul(k.ptr(k.st(S))))),
                                   k.blk({k.es(k.call(fr, {k.leaf(k.pn(v), 0)})), k.es(k.call(frs, {k.pn(v)})),
                                          k.asg(v, k.nul(k.ptr(k.st(S))))}))};
           const Stmt* bs {k.blkc({drop, k.asg(i, k.add(k.v(i), N(k, "1")))})};
           const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                                 {k.cl(k.range(k.v(i), k.i32, "0", "3")),
                                  k.cl(k.lor(k.lt(k.v(i), N(k, "3")), k.eq(k.pn(v), k.nul(k.ptr(k.st(S))))))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
           f->body =
               k.blkc({k.decl(v, k.call(ca, {})), k.iff(k.eq(k.pn(v), k.nul(k.ptr(k.st(S)))), k.blk({k.ret(nullptr)})),
                       k.decl(i, N(k, "0")), lp});
           k.done(f);
       });

    // (S2) struct Leaf { int *x (owning); int k; }  struct Mid { struct Leaf *b (owning); int m; }
    // int f(void) { struct Mid a = { NULL, 0 }; a.b = calloc(); if (a.b == NULL) { return 0; } int i = 0; while (i < 3) { [free(a.b->x);] a.b->x = malloc(); i++; }
    // free(a.b->x); free(a.b); a.b = NULL; }     the loop carries the chain a.b -> x: the closure of the leaf a.b is a synthetic site too
    const auto chain_loop = [=](Kit& k, bool free_first) -> SrcLoc
    {
        StructType* Leaf {k.strukt("Leaf", {{"x", PI(k)}, {"k", k.i32}})};
        Leaf->members[0U].owning = true;
        StructType* Mid {k.strukt("Mid", {{"b", k.ptr(k.st(Leaf))}, {"m", k.i32}})};
        Mid->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(Leaf), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* frl {heap_free_view(k, k.st(Leaf))};
        Function* f {k.fn("f", k.vd)};
        Var* a {k.scell(f, "a", Mid)};
        Var* i {k.local(f, "i", k.i32)};
        const auto ab = [&]()
        {
            return k.leaf(k.addr(a), 0);
        };
        std::vector<const Stmt*> body;
        if (free_first)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(ab(), 0)})));
        }
        body.push_back(k.lstore(ab(), 0, k.call(m, {})));
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* bs {k.blkc(body)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        f->body = k.blkc({k.decl(a, nullptr), k.lstore(k.addr(a), 0, k.nul(k.ptr(k.st(Leaf)))),
                          k.lstore(k.addr(a), 1, N(k, "0")), k.lstore(k.addr(a), 0, k.call(ca, {})),
                          k.iff(k.eq(ab(), k.nul(k.ptr(k.st(Leaf)))), k.blk({k.ret(nullptr)})), k.decl(i, N(k, "0")),
                          lp, k.es(k.call(fr, {k.leaf(ab(), 0)})), k.es(k.call(frl, {ab()})),
                          k.lstore(k.addr(a), 0, k.nul(k.ptr(k.st(Leaf))))});
        k.done(f);
        return bs->end_loc;
    };
    lc("owning_leaf_chain_loop",
       "a loop that frees and replaces a.b->x of a struct local's chain: the leaf a.b carries the closure of its pointee's owning field",
       [=](Kit& k)
       {
           chain_loop(k, true);
       });
    lc("owning_leaf_chain_loop_drop",
       "the same without the free: the loop's back-edge row reports the block it drops (leak at the end of the body)",
       [=](Kit& k)
       {
           k.fail_only("leak", chain_loop(k, false), 1U, 0U);
       });

    // (B2) struct S { int *q (owning); int n; }  struct S g;  void bump(int **pp) requires(owns(*pp)) ensures(owns(*pp) && *pp == old(*pp));
    // void f(void) requires(owns(g)) ensures(owns(g)) { int *z = NULL; int i = 0; while (i < 3) { bump(&z); i++; } [g.q = NULL;] }
    // the loop's callee names cells of g.q's type, so f (which holds g) carries the leaf by type: its block survives the loop and its ghost is not the loop head's havoc
    // (the twin's store of NULL still sees the block it drops)
    const auto type_loop = [=](Kit& k, bool drop) -> SrcLoc
    {
        StructType* S {mk_s(k)};
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
        Global* g {k.prog.add_global("g", k.st(S), k.at(), f)};
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
        k.req(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), true));
        k.req(f, g_closure());
        k.ens(f, k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), true));
        k.ens(f, g_closure());
        Var* z {k.cell(f, "z", PI(k))};
        Var* w {k.cell(f, "w", k.i32)};
        Var* i {k.local(f, "i", k.i32)};
        // (the store of an int makes the loop one that keeps the contents' flags: what begins initialized ends so)
        const Stmt* bs {k.blkc(
            {k.es(k.call(bump, {k.addr(z)})), k.store(k.addr(w), N(k, "1")), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
        std::vector<const Stmt*> body {k.decl(z, k.nul(PI(k))), k.decl(w, N(k, "0")), k.decl(i, N(k, "0")), lp};
        const Stmt* st {k.lstore(k.b.global_addr(g, k.at()), 0, k.nul(PI(k)))};
        if (drop)
        {
            body.push_back(st);
        }
        f->body = k.blkc(body);
        k.done(f);
        return st->loc;
    };
    lc("owning_global_leaf_type_loop",
       "f holds the owning leaf g.q across a loop whose callee names cells of its type: carried by type, its block still there for the ensures",
       [=](Kit& k)
       {
           type_loop(k, false);
       });
    lc("owning_global_leaf_type_loop_drop",
       "the same with g.q = NULL after the loop: the store reports the block it drops (the ghost survived the loop)",
       [=](Kit& k)
       {
           k.fail("leak", type_loop(k, true), 1U, "", "assigning to owning field");
       });

    // (B2) struct S { int *q (owning); int n; }  struct S g;  void f(void) { struct S s = { NULL, 0 }; int i = 0; while (i < 3) { free(s.q); s.q = malloc(); i++; } free(s.q); }
    // a global struct with an owning field that the function never names: its leaf is neither carried (the loop does not use it) nor reachable, so its ghost keeps its value
    // across the loop and no row reads its cell under the ghost alone
    lc("owning_global_leaf_unused_loop",
       "a loop over a struct local's owning leaf in a program that declares an unrelated struct global with an owning field: the global's ghost is framed out of the loop",
       [=](Kit& k)
       {
           StructType* S {mk_s(k)};
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
           Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
           Function* fr {heap_free_view(k, k.i32)};
           Function* f {k.fn("f", k.vd)};
           Var* s {k.scell(f, "s", S)};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* bs {k.blkc({k.es(k.call(fr, {k.leaf(k.addr(s), 0)})), k.lstore(k.addr(s), 0, k.call(m, {})),
                                   k.asg(i, k.add(k.v(i), N(k, "1")))})};
           const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))}, bs)};
           f->body =
               k.blkc({k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))), k.lstore(k.addr(s), 1, N(k, "0")),
                       k.decl(i, N(k, "0")), lp, k.es(k.call(fr, {k.leaf(k.addr(s), 0)}))});
           k.done(f);
       });
}

} // namespace
