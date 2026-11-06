// (included by selftest.cpp) The last lowering rules of decision 0052 (M4.5d2b1): an `owns_block` site whose guard
// reads the heap (item 6 (b)), a block of vlibc's allocator named in a
// contract, `cerastes_owns_vlibc_block` (item 7: `Expr::vlibc_block`, the token's value `false` in the rows, and a
// program that names one is a vlibc heap program), the use
// of a pointer in an argument beside a call that ends its block (item 6 (e): unsequenced, not a failed row of the
// call), and the ending row of a partnerless atom the
// function does not hold (item 2: one error, not one per cell).  Each rule has a case that verifies and a twin that
// fails.
namespace {

void define_block_guards_and_vlibc_sites()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto block = [](Kit& k, const Expr* ptr)
    {
        return k.b.atom(AtomKind::OwnsBlock, ptr, -1, k.at());
    };
    auto vblock = [](Kit& k, const Expr* ptr)
    {
        const Expr* a {k.b.atom(AtomKind::OwnsBlock, ptr, -1, k.at())};
        const_cast<Expr*>(a)->vlibc_block = true; // `cerastes_owns_vlibc_block(p)`
        return a;
    };
    auto owns = [](Kit& k, const Expr* ptr, int amount)
    {
        return k.b.atom(AtomKind::Owns, ptr, amount, k.at());
    };
    auto live = [](Kit& k, const Expr* ptr, int amount)
    {
        return k.b.atom(AtomKind::Live, ptr, amount, k.at());
    };
    auto notnull = [](Kit& k, const Var* p)
    {
        return k.ne(k.pn(p), k.nul(k.ptr(k.i32)));
    };
    constexpr Allocator V {Allocator::Vlibc};

    // ======== (b) a block site whose guard reads the heap ========
    // void f(int *q, int *p) requires(owns(*q) && (*q > 0 ? owns_block(p) : true)) ensures(owns(*q) && *q == old(*q)) {
    // if (*q > 0) [free(p);] }
    const auto guarded = [=](Kit& k, bool freed) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd, {{"q", PT(k)}, {"p", PT(k)}})};
        k.req(f, k.land(owns(k, FP(k, f, 0), 0),
                        k.cond(k.gt(k.deref(FP(k, f, 0)), N(k, "0")), block(k, FP(k, f, 1)), k.yes())));
        k.ens(f, k.land(owns(k, FP(k, f, 0), 0), k.eq(k.deref(FP(k, f, 0)), k.old(k.deref(FP(k, f, 0))))));
        std::vector<const Stmt*> in;
        if (freed)
        {
            in.push_back(k.es(k.call(fr, {k.pn(f->param_var(1))})));
        }
        f->body = k.blk({k.iff(k.gt(k.deref(k.pn(f->param_var(0))), N(k, "0")), k.blk(in))});
        k.done(f);
        return f->end_loc;
    };
    lc("bg_guard_requires",
       "requires(owns(*q) && (*q > 0 ? owns_block(p) : true)): the block is taken where *q > 0, read in the entry state, and freed under the same test",
       [=](Kit& k)
       {
           guarded(k, true);
       });
    lc("bg_guard_requires_leak",
       "the same without the free: the leak exemption holds no token, the block is held where *q > 0 (leak at the closing brace)",
       [=](Kit& k)
       {
           k.fail("leak", guarded(k, false));
       });

    // void g(int *q, int *p) requires(owns(*q) && (*q > 0 ? owns_block(p) : true)) ensures(owns(*q) && *q == old(*q) &&
    // (*q > 0 ? owns_block(p) : true)) { }
    // the block is borrowed where *q > 0 and returned under the guard read in the post-state; the exemption repeats
    // that guard
    lc("bg_guard_borrow",
       "a block borrowed under a guard that reads *q, returned under the same guard in the post-state: no leak",
       [=](Kit& k)
       {
           Function* g {k.fn("g", k.vd, {{"q", PT(k)}, {"p", PT(k)}})};
           const auto guard = [&]()
           {
               return k.cond(k.gt(k.deref(FP(k, g, 0)), N(k, "0")), block(k, FP(k, g, 1)), k.yes());
           };
           k.req(g, k.land(owns(k, FP(k, g, 0), 0), guard()));
           k.ens(g, k.land(k.land(owns(k, FP(k, g, 0), 0), k.eq(k.deref(FP(k, g, 0)), k.old(k.deref(FP(k, g, 0))))),
                           guard()));
           g->body = k.blk({});
           k.done(g);
       });

    // ======== item 7: `cerastes_owns_vlibc_block(p)` ========
    // void eat(int *p) requires(owns_vlibc_block(p)) { vlibc_free(p) | free(p) }   a consuming callee names vlibc's
    // block and frees it with vlibc's free
    const auto eat = [=](Kit& k, Allocator dealloc) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32, dealloc)};
        Function* f {k.fn("eat", k.vd, {{"p", PT(k)}})};
        k.req(f, vblock(k, FP(k, f, 0)));
        const Expr* c {k.call(fr, {k.pn(f->param_var(0))})};
        f->body = k.blk({k.es(c)});
        k.done(f);
        return c->loc;
    };
    lc("vs_consume",
       "requires(owns_vlibc_block(p)) and vlibc_free(p): the token's value false is what the site and vlibc_free state",
       [=](Kit& k)
       {
           eat(k, V);
       });
    lc("vs_consume_platform_free",
       "the same with the platform's free: the block is vlibc's and free requires the platform's (invalid-free at the value row)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", eat(k, Allocator::Platform), 5U, 1U);
       });

    // void touch(int *p) requires(owns_vlibc_block(p)) ensures(owns_vlibc_block(p)) { *p = 2; }
    // void use(void) { int *p = ALLOC(); if (p != NULL) { touch(p); vlibc_free(p); } }     the caller's block is
    // vlibc's (V) or the platform's (P: permission at the call)
    const auto borrow = [=](Kit& k, Allocator alloc) -> SrcLoc
    {
        Function* t {k.fn("touch", k.vd, {{"p", PT(k)}})};
        k.req(t, vblock(k, FP(k, t, 0)));
        k.ens(t, vblock(k, FP(k, t, 0)));
        t->body = k.blk({k.store(k.pn(t->param_var(0)), N(k, "2"))});
        k.done(t);
        Function* m {heap_alloc_view(k, alloc == V ? "vlibc_malloc" : "malloc", k.i32, false, alloc)};
        Function* fr {heap_free_view(k, k.i32, alloc)};
        Function* u {k.fn("use", k.vd)};
        Var* p {k.local(u, "p", PT(k))};
        const Expr* c {k.call(t, {k.pn(p)})};
        u->body =
            k.blk({k.decl(p, k.call(m, {})),
                   k.iff(notnull(k, p), k.blk({k.store(k.pn(p), N(k, "1")), k.es(c), k.es(k.call(fr, {k.pn(p)}))}))});
        k.done(u);
        return c->loc;
    };
    lc("vs_borrow", "a vlibc block borrowed by touch (owns_vlibc_block in both lists) and then freed by the caller",
       [=](Kit& k)
       {
           borrow(k, V);
       });
    lc("vs_borrow_platform_block",
       "the caller passes a platform block to the callee that names vlibc's: permission at the call (the value row, the second of its rows)",
       [=](Kit& k)
       {
           k.fail_only("permission", borrow(k, Allocator::Platform), 6U, 2U);
       });

    // ======== (e) `two((free(p), 0), p)` ========
    // int two(int a, int *q) requires(live(q)) ensures(live(q));
    // int use_two(void) { int *p = malloc(); if (p != NULL) { two((free(p), 0), p) | { free(p); two(0, p) } } }
    const auto beside = [=](Kit& k, bool same_expr) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* two {k.fn("two", k.vd, {{"a", k.i32}, {"q", PT(k)}})};
        k.req(two, live(k, FP(k, two, 1), 0));
        k.ens(two, live(k, FP(k, two, 1), 0));
        k.trusted(two);
        k.done(two);
        Function* f {k.fn("use_two", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        const Expr* use {k.pn(p)};
        std::vector<const Stmt*> in;
        if (same_expr)
        {
            in.push_back(k.es(k.call(two, {k.comma(k.call(fr, {k.pn(p)}), N(k, "0")), use})));
        }
        else
        {
            in.push_back(k.es(k.call(fr, {k.pn(p)})));
        }
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p), k.blk(in))});
        k.done(f);
        return use->loc;
    };
    lc("fu_sequenced", "free(p) in its own statement: nothing is unsequenced",
       [=](Kit& k)
       {
           beside(k, false);
       });
    lc("fu_unsequenced",
       "two((free(p), 0), p): the use of p as the second argument and the call that ends its block are unsequenced (not the call's own row)",
       [=](Kit& k)
       {
           k.fail("unsequenced", beside(k, true));
       });

    // ======== item 2: the ending row of a partnerless atom ========
    // int **hand(int **pp) requires(owns(*pp)) ensures(r, owns(*r) && owns(**r) && r == pp) { return pp; }      **r is
    // not held: ONE error, at the return
    lc("er_not_held_once",
       "owns(*r) is given (the callee holds *pp) and owns(**r) is not: the ending row fails once and the cell rows after it are not reached",
       [=](Kit& k)
       {
           const Type PP {k.ptr(PT(k))};
           Function* f {k.fn("hand", PP, {{"pp", PP}})};
           k.req(f, owns(k, FP(k, f, 0), 0));
           const Expr* r {k.b.text(k.R(f), "r")};
           k.ens(f, k.land(k.land(owns(k, r, 1), owns(k, k.deref(r), 2)), k.eq(r, FP(k, f, 0))));
           const Stmt* rt {k.ret(k.pn(f->param_var(0)))};
           f->body = k.blk({rt});
           k.done(f);
           k.fail_only("permission", rt->loc, 15U, 2U);
       });
}

}
