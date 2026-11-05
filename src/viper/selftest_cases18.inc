// (included by selftest.cpp) vlibc's allocator beside the platform's (decision 0050, M4.5d2v): the VIR the encoder builds for `vlibc_malloc`, `vlibc_calloc` and
// `vlibc_free`: the same typed views as malloc's, calloc's and free's with `Function::allocator == Vlibc`.  A block's allocator is the VALUE of its token
// (`true`: the platform's, `false`: vlibc's); a deallocation or an `owns_block` consumer requires its allocator's value at a row of its own, and a loop carries the
// value (item 5).  The ROW COUNTS and `fail_only` indexes of the failing cases are the lowering's to settle at its first run (the permission row of the token comes
// first, the value row second; a failed value row ends its path, so each twin reports one error).  Each rule has a case that verifies and a twin that fails.
namespace {

void define_vlibc_heap()
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
    auto notnull = [](Kit& k, const Var* p, Type t)
    {
        return k.ne(k.pn(p), k.nul(t));
    };
    constexpr Allocator V {Allocator::Vlibc};
    constexpr Allocator P {Allocator::Platform};

    // ======== one block, each allocator, each deallocator ========
    // void f(void) { int *p = ALLOC(); if (p != NULL) { *p = 5; DEALLOC(p); [DEALLOC(p);] } }
    //   `alloc` and `dealloc` name the allocator of each (V or P); `again`: a second deallocation of the same routine (invalid-free at it).
    //   The views present in the program are exactly the ones the call uses (the program of 0050 audit B1: `P` malloc with `V` free has no vlibc ALLOCATION view).
    const auto life = [=](Kit& k, Allocator alloc, Allocator dealloc, bool again, bool leak) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, alloc == V ? "vlibc_malloc" : "malloc", k.i32, false, alloc)};
        Function* fr {heap_free_view(k, k.i32, dealloc)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        std::vector<const Stmt*> in {k.store(k.pn(p), N(k, "5"))};
        SrcLoc where;
        if (!leak)
        {
            const Expr* first {k.call(fr, {k.pn(p)})};
            where = first->loc;
            in.push_back(k.es(first));
        }
        if (again)
        {
            const Expr* second {k.call(fr, {k.pn(p)})};
            where = second->loc;
            in.push_back(k.es(second));
        }
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk(in))});
        k.done(f);
        return leak ? f->end_loc : where;
    };
    lc("vl_life",
       "p = vlibc_malloc(); *p = 5; vlibc_free(p): the block's value is false and vlibc_free requires false; nothing leaks",
       [=](Kit& k)
       {
           life(k, V, V, false, false);
       });
    lc("vl_life_platform",
       "the same with malloc and free in a program that also has the vlibc views (the platform's rows hold: the value is true)",
       [=](Kit& k)
       {
           // present, unused: the program is a vlibc heap program
           Function* vm {heap_alloc_view(k, "vlibc_malloc", k.i32, false, V)};
           (void)vm;
           life(k, P, P, false, false);
       });
    lc("vl_free_vlibc_with_platform",
       "p = vlibc_malloc(); free(p): the block is vlibc's and free requires the platform's (invalid-free at the value row: the second of the five rows of a free of an int)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", life(k, V, P, false, false), 5U, 1U);
       });
    lc("vl_free_platform_with_vlibc",
       "p = malloc(); vlibc_free(p): the block is the platform's and vlibc_free requires vlibc's (invalid-free at the value row); the program holds no vlibc allocation view (audit B1)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", life(k, P, V, false, false), 5U, 1U);
       });
    lc("vl_double_free",
       "vlibc_free(p) twice: the second finds no token (invalid-free at the permission row, the first row)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", life(k, V, V, true, false), 5U, 0U);
       });
    lc("vl_leak", "p = vlibc_malloc() and never vlibc_free: a vlibc block still held at the end is a leak",
       [=](Kit& k)
       {
           k.fail("leak", life(k, V, V, false, true));
       });

    // ======== a loop carries the block's allocator ========
    // void f(void) { int *p = ALLOC(); if (p != NULL) { *p = 0; int i = 0; while (i < 3) { [BODY] i++; } vlibc_free(p); } }
    //   mode 0: the body does not name p: the token place keeps the value (the invariant `tok == ktv`); verifies
    //   mode 1: the body frees p (a block the loop held): the back edge's rows lose the cells and the token (permission at the loop)
    const auto vloop = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* vm {heap_alloc_view(k, "vlibc_malloc", k.i32, false, V)};
        Function* fr {heap_free_view(k, k.i32, V)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        SrcLoc where;
        if (mode == 1)
        {
            body.push_back(k.es(k.call(fr, {k.pn(p)})));
        }
        body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blk(body))};
        where = lp->loc;
        f->body =
            k.blk({k.decl(p, k.call(vm, {})), k.decl(i, N(k, "0")),
                   k.iff(notnull(k, p, PT(k)), k.blk({k.store(k.pn(p), N(k, "0")), lp, k.es(k.call(fr, {k.pn(p)}))}))});
        k.done(f);
        return where;
    };
    lc("vl_loop_carry",
       "a loop that does not name p carries the vlibc block (its token value false, snapshot ktv): vlibc_free(p) after it verifies",
       [=](Kit& k)
       {
           vloop(k, 0);
       });
    lc("vl_loop_free_in_body",
       "the body frees the vlibc block the loop holds: the back edge's rows fail (permission) before any internal row, never exit 3",
       [=](Kit& k)
       {
           // the first row (the cells' permission) fails and ends the path; the token row behind it is the same loss
           k.fail_only("permission", vloop(k, 1), 6U, 0U);
       });

    // ======== owns_block consumers take the platform's allocator ========
    // void eat(int *p) requires(owns_block(p)) [trusted];  void f(void) { int *p = ALLOC(); if (p != NULL) { *p = 0; eat(p); } }
    const auto consume = [=](Kit& k, Allocator alloc) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, alloc == V ? "vlibc_malloc" : "malloc", k.i32, false, alloc)};
        Function* vf {heap_free_view(k, k.i32, V)}; // makes the program a vlibc heap program whichever the allocation
        (void)vf;
        Function* eat {k.fn("eat", k.vd, {{"p", PT(k)}})};
        k.req(eat, block(k, FP(k, eat, 0)));
        k.trusted(eat);
        k.done(eat);
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        const Expr* call {k.call(eat, {k.pn(p)})};
        f->body = k.blk(
            {k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk({k.store(k.pn(p), N(k, "0")), k.es(call)}))});
        k.done(f);
        return call->loc;
    };
    lc("vl_consume_platform",
       "eat(p) of a platform block: owns_block means the platform's allocator and the value row holds",
       [=](Kit& k)
       {
           consume(k, P);
       });
    lc("vl_consume_vlibc",
       "eat(p) of a vlibc block: the value row fails (permission: a contract cannot name vlibc's blocks yet; the third of the six permission rows of the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", consume(k, V), 6U, 2U);
       });

    // int *make(void) ensures(r == NULL || owns_block(r)): a vlibc block returned through owns_block fails its value row at the return
    const auto give = [=](Kit& k, Allocator alloc) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, alloc == V ? "vlibc_malloc" : "malloc", k.i32, false, alloc)};
        Function* vf {heap_free_view(k, k.i32, V)};
        (void)vf;
        Function* f {k.fn("make", PT(k))};
        k.ens(f, k.lor(k.eq(k.R(f), k.nul(PT(k))), block(k, k.R(f))));
        Var* p {k.local(f, "p", PT(k))};
        const Stmt* r {k.ret(k.pn(p))};
        f->body =
            k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk({k.store(k.pn(p), N(k, "0"))})), r});
        k.done(f);
        return r->loc; // the return rows are at the return statement
    };
    lc("vl_give_platform", "make() returns a platform block through owns_block(r): the value row holds",
       [=](Kit& k)
       {
           give(k, P);
       });
    lc("vl_give_vlibc",
       "make() returns a vlibc block through owns_block(r): the value row fails at the return (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", give(k, V), 6U, 2U);
       });
}

} // namespace
