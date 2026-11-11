// (included by selftest.cpp) `cerastes::ref_to_uninit` on an owning field, Part F (decision 0058 item 4, M4.5d2b2d): the closure site of an owning field that is also
// `ref_to_uninit` is a marked block (0047 item 8's mode, as the deep atom of an owning and marked global in 0054 item 2): a `requires` site claims no flag, a borrowed
// `ensures` site keeps a set flag, and an unmarked owning field keeps the plain closure.  `StructMember::ref_to_uninit` is set by hand here (the encoder sets it from
// the attribute).  A given site (the result's closure) is a test of the encoder: tests/memory/*marked_field*.c.
namespace {

void define_marked_fields()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    // struct S { int *q (owning [, ref_to_uninit]); int n; }
    auto mk_s = [](Kit& k, bool owning, bool marked)
    {
        StructType* S {k.strukt("S", {{"q", k.ptr(k.i32)}, {"n", k.i32}})};
        S->members[0U].owning = owning;
        S->members[0U].ref_to_uninit = marked;
        return S;
    };
    // void fill(struct S *p) requires(owns(*p) && (p->q == NULL || owns_block(p->q)))
    //                        ensures(owns(*p) && p->q == old(p->q) && (p->q == NULL || owns_block(p->q)))
    //   { if (p->q != NULL) { [int x = *p->q;] *p->q = 7; } }
    // `reads_first` puts the read first; the location of the read is returned through `rd`.  `tie`: the contract says `p->q == old(p->q)` (else it does not);
    // `replace`: the body frees p->q and stores a new block instead of filling it:
    //   { Pointer t = malloc(); free(p->q); p->q = NULL; if (t != NULL) { *t = 7; p->q = t; } }
    auto mk_fill = [=](Kit& k, StructType* S, bool reads_first, const Expr** rd, bool tie = true, bool replace = false) -> Function*
    {
        Function* fill {k.fn("fill", k.vd, {{"p", k.ptr(k.st(S))}})};
        const auto atoms = [&](bool tie)
        {
            const Expr* pq {k.b.text(k.leaf(k.F(fill, 0), 0), "p->q")};
            const Expr* own {k.b.atom(AtomKind::Owns, k.b.text(k.F(fill, 0), "p"), 0, k.at())};
            const Expr* close {k.lor(k.eq(k.leaf(k.F(fill, 0), 0), k.nul(PI(k))), k.b.atom(AtomKind::OwnsBlock, pq, -1, k.at()))};
            if (!tie)
            {
                return k.land(own, close);
            }
            return k.land(k.land(own, k.eq(k.leaf(k.F(fill, 0), 0), k.old(k.leaf(k.F(fill, 0), 0)))), close);
        };
        k.req(fill, atoms(false));
        k.ens(fill, atoms(tie));
        std::vector<const Stmt*> in;
        if (reads_first)
        {
            Var* x {k.local(fill, "x", k.i32)};
            *rd = k.deref(k.leaf(k.pv(fill, 0), 0));
            in.push_back(k.decl(x, *rd));
        }
        if (replace)
        {
            Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
            Function* fr {heap_free_view(k, k.i32)};
            Var* t {k.local(fill, "t", PI(k))};
            fill->body = k.blkc({k.decl(t, k.call(m, {})), k.es(k.call(fr, {k.leaf(k.pv(fill, 0), 0)})),
                                 k.lstore(k.pv(fill, 0), 0, k.nul(PI(k))),
                                 k.iff(k.ne(k.pn(t), k.nul(PI(k))),
                                       k.blk({k.store(k.pn(t), N(k, "7")), k.lstore(k.pv(fill, 0), 0, k.pn(t))}))});
            k.done(fill);
            return fill;
        }
        in.push_back(k.store(k.leaf(k.pv(fill, 0), 0), N(k, "7")));
        fill->body = k.blk({k.iff(k.ne(k.leaf(k.pv(fill, 0), 0), k.nul(PI(k))), k.blkc(in))});
        k.done(fill);
        return fill;
    };
    // int use(void) { struct S s; s.q = NULL; s.n = 0; s.q = malloc(); [if (s.q != NULL) *s.q = 5;] fill(&s);
    //                 [if (s.q != NULL) { int y = *s.q; }] free(s.q); return 0; }
    // `before`: the caller writes the buffer before the call; `after`: it reads it after the call.  Returns the location of the call and of the read.
    struct Where
    {
        SrcLoc call;
        SrcLoc read;
    };
    auto mk_use = [=](Kit& k, Function* fill, bool before, bool after) -> Where
    {
        StructType* S {const_cast<StructType*>(fill->params[0U].type.pointee->struct_type)};
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* use {k.fn("use", k.i32)};
        Var* s {k.scell(use, "s", S)};
        Var* y {k.local(use, "y", k.i32)};
        std::vector<const Stmt*> body {k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.nul(PI(k))),
                                       k.lstore(k.addr(s), 1, N(k, "0")), k.lstore(k.addr(s), 0, k.call(m, {}))};
        if (before)
        {
            body.push_back(k.iff(k.ne(k.leaf(k.addr(s), 0), k.nul(PI(k))),
                                 k.blk({k.store(k.leaf(k.addr(s), 0), N(k, "5"))})));
        }
        const Expr* call {k.call(fill, {k.addr(s)})};
        body.push_back(k.es(call));
        Where w;
        w.call = call->loc;
        if (after)
        {
            const Expr* rd {k.deref(k.leaf(k.addr(s), 0))};
            w.read = rd->loc;
            body.push_back(k.iff(k.ne(k.leaf(k.addr(s), 0), k.nul(PI(k))), k.blk({k.decl(y, rd)})));
        }
        body.push_back(k.es(k.call(fr, {k.leaf(k.addr(s), 0)})));
        body.push_back(k.ret(N(k, "0")));
        use->body = k.blkc(body);
        k.done(use);
        return w;
    };
    lc("mf_filler_ok",
       "an owning and ref_to_uninit field: an allocated, unwritten buffer is passed to a filler that writes it: the closure's requires site claims no flag",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           mk_use(k, mk_fill(k, mk_s(k, true, true), false, &rd), false, false);
       });
    lc("mf_filler_reads_first",
       "the filler reads *p->q before writing it: the requires site claimed no flag, so the read is uninitialized (in the callee)",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           Function* fill {mk_fill(k, mk_s(k, true, true), true, &rd)};
           mk_use(k, fill, false, false);
           k.fail("uninitialized", rd->loc);
       });
    lc("mf_unmarked_plain",
       "the same field without the marker: the plain closure claims the flag, and the call of the filler is uninitialized",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           Function* fill {mk_fill(k, mk_s(k, true, false), false, &rd)};
           // three rows stand at the call (the leaves s.q and s.n of the struct local, then the closure's cell); the last one fails
           k.fail_only("uninitialized", mk_use(k, fill, false, false).call, 3U, 2U);
       });
    lc("mf_nonowning_plain",
       "a marked field that does not own: a written owns_block site on it is plain (only an owning and marked field's closure is the marked mode): the call is uninitialized",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           Function* fill {mk_fill(k, mk_s(k, false, true), false, &rd)};
           // three rows stand at the call (the leaves s.q and s.n of the struct local, then the closure's cell); the last one fails
           k.fail_only("uninitialized", mk_use(k, fill, false, false).call, 3U, 2U);
       });
    lc("mf_keeps_flag",
       "the caller wrote the buffer before the call and reads it after: the borrowed ensures site keeps the set flag (old(flag) ==> flag)",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           mk_use(k, mk_fill(k, mk_s(k, true, true), false, &rd), true, true);
       });
    lc("mf_caller_reads_unknown",
       "the caller never wrote the buffer and reads it after the filler: the ensures promises nothing about the flag, the read is uninitialized",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           Function* fill {mk_fill(k, mk_s(k, true, true), false, &rd)};
           k.fail("uninitialized", mk_use(k, fill, false, true).read);
       });
    lc("mf_untied_replaces",
       "the contract does not tie p->q and the body frees it and stores a new block: the site claims no flag (an untied site is the unflagged mode), so the contract holds (audit B1: no row compares two blocks)",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           mk_use(k, mk_fill(k, mk_s(k, true, true), false, &rd, false, true), false, false);
       });
    lc("mf_untied_replaces_caller_reads",
       "the same, and the caller wrote the buffer before the call and reads it after: the untied site promised no flag, so the read is uninitialized (at the caller)",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           Function* fill {mk_fill(k, mk_s(k, true, true), false, &rd, false, true)};
           k.fail("uninitialized", mk_use(k, fill, true, true).read);
       });
    lc("mf_tied_replaces",
       "the contract ties p->q and the body replaces it: ONE postcondition (the clause's own assertion, tie and kept row together), no check of Silicon's own that fails after it",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           Function* fill {mk_fill(k, mk_s(k, true, true), false, &rd, true, true)};
           mk_use(k, fill, false, false);
           k.fail("postcondition", fill->end_loc);
       });
    // ---- a marked owner closes nothing (question 202), in a loop's synthetic closures (lower.cpp make_closure's three call sites) ----
    // struct Leaf { int *x (owning); int k; }  static owning [ref_to_uninit] struct Leaf *gp;
    // int main(void) { gp = calloc(1, sizeof(struct Leaf)); if (gp == NULL) { return 1; } int i = 0;
    //                  while (i < 3) { [free(gp->x); gp->x = malloc(sizeof(int));] | [gp->k = 1;] i++; } return 0; }
    const auto gp_loop = [=](Kit& k, bool marked, bool replace, SrcLoc* free_at = nullptr) -> SrcLoc
    {
        StructType* Leaf {k.strukt("Leaf", {{"x", PI(k)}, {"k", k.i32}})};
        Leaf->members[0U].owning = true;
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(Leaf), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("main", k.i32)};
        k.prog.set_entry(f);
        Global* gp {k.prog.add_global("gp", k.ptr(k.st(Leaf)), k.at())};
        gp->init_kind = Global::Init::Null;
        gp->owning = true;
        gp->ref_to_uninit = marked;
        const auto gv = [&]()
        {
            return k.deref(k.b.global_addr(gp, k.at()));
        };
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        if (replace)
        {
            const Expr* fcall {k.call(fr, {k.leaf(gv(), 0)})};
            if (free_at != nullptr)
            {
                *free_at = fcall->loc;
            }
            body.push_back(k.es(fcall));
            body.push_back(k.lstore(gv(), 0, k.call(m, {})));
        }
        else
        {
            body.push_back(k.lstore(gv(), 1, N(k, "1")));
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
    lc("mv_global_loop_unmarked_replaces",
       "the control: an unmarked owning struct pointer global, a loop that frees and replaces gp->x: the closure of gp's owning field is carried, as loop_global_pointer_closure",
       [=](Kit& k)
       {
           gp_loop(k, false, true);
       });
    lc("mv_global_loop_marked_untouched",
       "a ref_to_uninit owning struct pointer global, a loop that writes only gp->k: the marked site closes nothing, and the loop needs nothing it did not carry",
       [=](Kit& k)
       {
           gp_loop(k, true, false);
       });
    // void f(int n) { struct S *v = calloc(1, sizeof(struct S)); [owning, ref_to_uninit] int i = 0; while (i < n) { if (v != NULL) { v->n = 1; } i++; } if (v != NULL) { free(v->q); free(v); } }
    // the loop touches only the value cell n: a marked owner carries no closure of v->q, and the free of v->q after the loop is of a block the function still holds
    const auto local_loop = [=](Kit& k, bool marked) -> SrcLoc
    {
        StructType* S {mk_s(k, true, false)};
        Function* ca {heap_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* frs {heap_free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd, {{"n", k.i32}})};
        k.req(f, k.range(k.F(f, 0), k.i32, "0", "3"));
        Var* v {k.local(f, "v", k.ptr(k.st(S)))};
        v->owning = true;
        v->ref_to_uninit = marked;
        Var* i {k.local(f, "i", k.i32)};
        const Expr* nn {k.ne(k.pn(v), k.nul(k.ptr(k.st(S))))};
        const Stmt* bs {k.blkc({k.iff(nn, k.blk({k.lstore(k.pn(v), 1, N(k, "1"))})), k.asg(i, k.add(k.v(i), N(k, "1")))})};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, bs)};
        f->body = k.blkc({k.decl(v, k.call(ca, {})), k.decl(i, N(k, "0")), lp,
                          k.iff(k.ne(k.pn(v), k.nul(k.ptr(k.st(S)))),
                                k.blk({k.es(k.call(fr, {k.leaf(k.pn(v), 0)})), k.es(k.call(frs, {k.pn(v)}))}))});
        k.done(f);
        return f->end_loc;
    };
    lc("mv_local_loop_unmarked_touches_n",
       "the control: an unmarked owning struct pointer local, a loop that writes only v->n, then free(v->q) and free(v)",
       [=](Kit& k)
       {
           local_loop(k, false);
       });
    lc("mv_local_loop_marked_touches_n",
       "the same local marked ref_to_uninit: the loop carries no closure of v->q, and the frees after it hold",
       [=](Kit& k)
       {
           local_loop(k, true);
       });
    // the backstop of check_program: a marker the encoder would have rejected
    rc("mf_backstop_non_pointer", "is not a pointer to an object",
       [=](Kit& k)
       {
           StructType* S {k.strukt("S", {{"n", k.i32}})};
           S->members[0U].ref_to_uninit = true;
       });
}

} // namespace
