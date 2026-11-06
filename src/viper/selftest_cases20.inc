// (included by selftest.cpp) Owning automatic locals (decision 0052 item 3, M4.5d2b1): `Var::owning` on a pointer local
// holds the block it points to.  The ghosts `held_v`
// (the function holds the block and `v` owns it) and `tv_v` (its token's value), the value row at every initialization
// and assignment (the new value is the start of a block
// the function holds whole), `held_v` kept by every call and free, and the ENDING ROWS at a scope's end, a `return` and
// before a reassignment: `if (!(held_v ==> owners)) {
// assert false }` (`leak`, one error per block).  The cases build malloc's and free's views as the encoder does
// (`heap_alloc_view`, `heap_free_view`).
namespace {

void define_owning_locals()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto notnull = [](Kit& k, const Var* p)
    {
        return k.ne(k.pn(p), k.nul(k.ptr(k.i32)));
    };
    auto owning_local = [](Kit& k, Function* f, const char* name)
    {
        Var* v {k.local(f, name, k.ptr(k.i32))};
        v->owning = true;
        return v;
    };

    // int f() { owning int *v = malloc(); if (v != NULL) { *v = 1; [free(v);] } return 0; }
    const auto life = [=](Kit& k, bool freed) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.i32)};
        Var* v {owning_local(k, f, "v")};
        std::vector<const Stmt*> in {k.store(k.pn(v), N(k, "1"))};
        if (freed)
        {
            in.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        const Stmt* body {k.blkc({k.decl(v, k.call(m, {})), k.iff(notnull(k, v), k.blk(in)), k.ret(N(k, "0"))})};
        f->body = body;
        k.done(f);
        return body->end_loc;
    };
    lc("ol_freed",
       "an owning local freed on the path where it holds a block: the ending rows at the return and at the closing brace hold",
       [=](Kit& k)
       {
           life(k, true);
       });
    lc("ol_leak_return", "the same without the free: the owner still holds its block at the return (leak)",
       [=](Kit& k)
       {
           life(k, false);
           k.fail("leak", k.prog.functions().back().body->stmts.back()->loc);
       });

    // void g() { owning int *v = malloc(); [free(v);] v = malloc(); free(v); }    (the reassignment drops the block
    // unless it was freed)
    const auto reassign = [=](Kit& k, bool freed_first) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("g", k.vd)};
        Var* v {owning_local(k, f, "v")};
        const Stmt* again {k.asg(v, k.call(m, {}))};
        std::vector<const Stmt*> st {k.decl(v, k.call(m, {}))};
        if (freed_first)
        {
            st.push_back(k.es(k.call(fr, {k.pn(v)})));
        }
        st.push_back(again);
        st.push_back(k.es(k.call(fr, {k.pn(v)})));
        f->body = k.blkc(st);
        k.done(f);
        return again->loc;
    };
    lc("ol_reassign_freed", "free(v) before v = malloc(): the old block is gone, the reassignment row holds",
       [=](Kit& k)
       {
           reassign(k, true);
       });
    lc("ol_reassign_leak", "v = malloc() while v holds its block: the reassignment drops it (leak at the assignment)",
       [=](Kit& k)
       {
           k.fail("leak", reassign(k, false));
       });

    // int *mk() ensures(r == NULL || owns_block(r)) { owning int *v = malloc(); return [v|NULL]; }   (the result is an
    // owner when the function has a block site on it)
    const auto make = [=](Kit& k, bool null) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* f {k.fn("mk", PT(k))};
        k.ens(f, k.lor(k.eq(k.b.text(k.R(f), "r"), k.nul(PT(k))),
                       k.b.atom(AtomKind::OwnsBlock, k.b.text(k.R(f), "r"), -1, k.at())));
        Var* v {owning_local(k, f, "v")};
        const Stmt* r {k.ret(null ? k.nul(PT(k)) : k.pn(v))};
        f->body = k.blkc({k.decl(v, k.call(m, {})), k.iff(notnull(k, v), k.blk({k.store(k.pn(v), N(k, "1"))})), r});
        k.done(f);
        return r->loc;
    };
    lc("ol_returned",
       "the owning local is returned by a function whose result is an owning block: v == result at the return",
       [=](Kit& k)
       {
           make(k, false);
       });
    lc("ol_return_other", "return NULL while v holds its block: the owner outlives nothing (leak at the return)",
       [=](Kit& k)
       {
           k.fail("leak", make(k, true));
       });

    // void h(int *p) requires(owns_block(p)) ensures(owns_block(p)) { owning int *v = p; }   (a borrowed block parked
    // in an owning local, audit S11)
    // twin: owning int *v = &x for a cell x: the value rule (the function holds no block there)
    const auto parked = [=](Kit& k, bool cell) -> SrcLoc
    {
        Function* f {k.fn("h", k.vd, {{"p", PT(k)}})};
        const Expr* pf {k.b.text(k.F(f, 0), "p")};
        k.req(f, k.b.atom(AtomKind::OwnsBlock, pf, -1, k.at()));
        k.ens(f, k.b.atom(AtomKind::OwnsBlock, k.b.text(k.F(f, 0), "p"), -1, k.at()));
        Var* v {owning_local(k, f, "v")};
        const Stmt* d;
        if (cell)
        {
            Var* x {k.cell(f, "x", k.i32)};
            d = k.decl(v, k.addr(x));
            f->body = k.blkc({k.decl(x, N(k, "1")), d});
        }
        else
        {
            d = k.decl(v, k.pv(f, 0));
            f->body = k.blkc({d});
        }
        k.done(f);
        return d->loc;
    };
    lc("ol_parked",
       "a borrowed block (a formal with an ensures block site) parked in an owning local: the formal is an owner that outlives the scope",
       [=](Kit& k)
       {
           parked(k, false);
       });
    lc("ol_value_rule",
       "owning int *v = &x of a cell: the function holds no block there, the value row fails (permission)",
       [=](Kit& k)
       {
           k.fail("permission", parked(k, true));
       });

    // void two() { owning int *a = malloc(); owning int *b = a; free(b); }    (a block handed from one owner to
    // another; freed once)
    const auto two = [=](Kit& k, bool freed) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("two", k.vd)};
        Var* a {owning_local(k, f, "a")};
        Var* b {owning_local(k, f, "b")};
        std::vector<const Stmt*> st {k.decl(a, k.call(m, {})), k.decl(b, k.pn(a))};
        if (freed)
        {
            st.push_back(k.es(k.call(fr, {k.pn(b)})));
        }
        const Stmt* body {k.blkc(st)};
        f->body = body;
        k.done(f);
        return body->end_loc;
    };
    lc("ol_handed",
       "a block handed from owner a to owner b and freed through b: a's block is gone (held_a is updated by the free)",
       [=](Kit& k)
       {
           two(k, true);
       });
    lc("ol_handed_leak",
       "the same without the free: b's exit row passes (a still owns it), a's exit row fails (leak at the closing brace)",
       [=](Kit& k)
       {
           k.fail_only("leak", two(k, false), 2U, 1U);
       });
}

} // namespace
