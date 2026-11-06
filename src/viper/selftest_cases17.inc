// (included by selftest.cpp) The heap (decision 0047, M4.5d2a): the VIR the encoder builds for vlibc's allocation functions.  A malloc or calloc call
// converted to `T *` is a call of a bodiless TYPED VIEW (no parameters, result `Ptr(T)`, `ensures r == NULL || owns_block(r)`, the flags unknown for malloc
// through `result_init`, zero value clauses for calloc), a `free(p)` is a call of a view `(Ptr(T)) -> void` with `requires p == NULL || owns_block(p)` and
// `deallocates` (items 2, 4); the checks are the free rows (`invalid-free`, item 6) and the leak row at every normal exit (`leak`, item 7).  The cases build
// those views by hand as the encoder does (`alloc_view`, `free_view`), so a lowering that handles them handles the encoder's.  Their tags carry the
// encoder's `; typed view: one <T>` (without the size, which the cases do not know), so `lint_viper.py` pins their text as it pins the encoder's views.
//
// The categories and the node each position names come from the record: a free's rows at the call, a leak at the `return` (or the closing brace of a function
// that falls off its end), `null-dereference` / `permission` / `uninitialized` at the store or the check that reads, `unsequenced` at the later-created access.
// The ROW COUNTS and `fail_only` indices of the failing cases are the lowering's to settle (the record does not fix how many rows a free has): they start at
// one failing check each and are fixed once the lowering runs.  Each rule has a case that verifies and a twin that fails for one reason (invariant 2).
namespace {

// malloc's or calloc's view for one object of `pointee`: `Ptr(T) malloc(void) ensures(r == NULL || owns_block(r) [&& every leaf is zero])`.
Function* heap_alloc_view(Kit& k, const char* name, Type pointee, bool zeroed, Allocator al = Allocator::Platform)
{
    const Type pt {k.ptr(pointee)};
    Function* f {k.fn(name, pt)};
    f->allocator = al; // decision 0050 item 3: a `vlibc_` view's token has the value false
    k.trusted(f, std::string("vlibc: ") + name + " returns NULL or one new block; typed view: one " + pointee.str() +
                     ", decision 0047");
    const Expr* is_null {k.eq(k.R(f), k.nul(pt))};
    const Expr* rhs {k.b.atom(AtomKind::OwnsBlock, k.R(f), -1, k.at())};
    auto zero_of = [&](const Type& t) -> const Expr*
    {
        return t.is_bool() ? k.no() : t.is_ptr() ? k.nul(t) : k.num(t, "0");
    };
    if (zeroed)
    {
        if (pointee.is_struct())
        {
            for (const StructLeaf& lf : struct_leaves(*pointee.struct_type))
            {
                const Expr* addr {k.R(f)};
                for (int idx : lf.path)
                {
                    addr = k.mem(addr, idx);
                }
                rhs = k.land(rhs, k.eq(k.deref(addr), zero_of(lf.type)));
            }
        }
        else
        {
            rhs = k.land(rhs, k.eq(k.deref(k.R(f)), zero_of(pointee)));
        }
    }
    else
    {
        f->result_init = InitMarker::RefToUninit;
    }
    k.ens(f, k.lor(is_null, rhs));
    k.done(f);
    return f;
}

// free's view for a pointer to `pointee`: `void free(T *ptr) requires(ptr == NULL || owns_block(ptr))`, `deallocates`, the parameter `ref_to_uninit`.
Function* heap_free_view(Kit& k, Type pointee, Allocator al = Allocator::Platform)
{
    const Type pt {k.ptr(pointee)};
    Function* f {k.fn(al == Allocator::Vlibc ? "vlibc_free" : "free", k.vd, {{"ptr", pt}})};
    f->allocator = al;
    f->params[0U].init = InitMarker::RefToUninit;
    k.trusted(f, std::string("vlibc: ") + f->c_name + " ends the allocation" +
                     (al == Allocator::Vlibc ? " in vlibc's heap" : "") + "; typed view: one " + pointee.str() +
                     ", decision 0047"); // the tag names the routine as the lint pins it
    k.req(f, k.lor(k.eq(k.b.text(k.F(f, 0), "ptr"), k.nul(pt)),
                   k.b.atom(AtomKind::OwnsBlock, k.b.text(k.F(f, 0), "ptr"), -1, k.at())));
    f->deallocates = true;
    k.done(f);
    return f;
}

void define_heap()
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
    auto owns = [](Kit& k, const Expr* place, int amount)
    {
        return k.b.atom(AtomKind::Owns, place, amount, k.at());
    };
    auto live = [](Kit& k, const Expr* ptr, int amount)
    {
        return k.b.atom(AtomKind::Live, ptr, amount, k.at());
    };
    auto notnull = [](Kit& k, const Var* p, Type t)
    {
        return k.ne(k.pn(p), k.nul(t));
    };

    // ======== the views: malloc, then use, then free ========
    // void f(void) { int *p = malloc(); [if (p != NULL)] { [*p = 5;] check(*p == 5); free(p); [free(p) | *p = 6 | check(p == q)] } }
    //   mode 0 verifies; 1: no null test (null-dereference at the store); 2: the read before any write (uninitialized at the check);
    //   3: a second free (invalid-free at it); 4: a write after the free (permission at the store); 5: p == q after the free (lifetime at the use of p)
    const auto int_life = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        Var* q {k.local(f, "q", PT(k))};
        SrcLoc where;
        std::vector<const Stmt*> in;
        if (mode != 2)
        {
            const Stmt* w {k.store(k.pn(p), N(k, "5"))};
            if (mode == 1)
            {
                where = w->loc;
            }
            in.push_back(w);
            in.push_back(k.chk(k.eq_c(k.deref(k.pn(p)), k.i32, "5")));
        }
        else
        {
            // the read before any write, into a local nothing checks: the one error is the read of the cell with no value
            Var* x {k.local(f, "x", k.i32)};
            const Expr* rd {k.deref(k.pn(p))};
            where = rd->loc;
            in.push_back(k.decl(x, rd));
        }
        in.push_back(k.es(k.call(fr, {k.pn(p)})));
        if (mode == 3)
        {
            const Expr* again {k.call(fr, {k.pn(p)})};
            where = again->loc;
            in.push_back(k.es(again));
        }
        else if (mode == 4)
        {
            const Stmt* w {k.store(k.pn(p), N(k, "6"))};
            where = w->loc;
            in.push_back(w);
        }
        else if (mode == 5)
        {
            const Expr* use {k.pn(p)};
            where = use->loc;
            in.push_back(k.chk(k.eq(use, k.pn(q))));
        }
        f->body = k.blk({k.decl(p, k.call(m, {})), k.decl(q, k.nul(PT(k))),
                         mode == 1 ? k.blk(in) : k.iff(notnull(k, p, PT(k)), k.blk(in))});
        k.done(f);
        return where;
    };
    lc("hv_int",
       "p = malloc(); if (p != NULL) { *p = 5; check(*p == 5); free(p); }: the view gives a block, free takes it, nothing leaks",
       [=](Kit& k)
       {
           int_life(k, 0);
       });
    lc("hv_int_no_null_test", "no null test: the store applies * to a pointer that may be null",
       [=](Kit& k)
       {
           k.fail("null-dereference", int_life(k, 1));
       });
    lc("hv_int_read_before_write",
       "malloc's contents are indeterminate (result_init): `int x = *p` before any write reads a cell with no value (uninitialized at the read)",
       [=](Kit& k)
       {
           k.fail("uninitialized", int_life(k, 2));
       });
    lc("hv_int_double_free",
       "free(p) twice: the second one finds no token (invalid-free: may already be freed; the token is row 0 of the four rows of a free of an int)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", int_life(k, 3), 4U, 0U);
       });
    lc("hv_int_write_after_free", "*p = 6 after free(p): the cell is gone (permission)",
       [=](Kit& k)
       {
           k.fail("permission", int_life(k, 4));
       });
    lc("hv_int_compare_after_free", "p == q after free(p): p's lifetime ended (lifetime at the use of p)",
       [=](Kit& k)
       {
           k.fail("lifetime", int_life(k, 5));
       });

    // struct S { int x; int y; };  void f(void) { struct S *s = malloc(); if (s != NULL) { s->x = 1; [s->y = 2;] check(s->x + s->y == 3); free(s); } }
    const auto struct_life = [=](Kit& k, bool skip_y) -> SrcLoc
    {
        StructType* S {k.strukt("S", {{"x", k.i32}, {"y", k.i32}})};
        const Type SP {k.ptr(k.st(S))};
        Function* m {heap_alloc_view(k, "malloc", k.st(S), false)};
        Function* fr {heap_free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* s {k.local(f, "s", SP)};
        std::vector<const Stmt*> in {k.lstore(k.pn(s), 0, N(k, "1"))};
        SrcLoc where;
        if (!skip_y)
        {
            in.push_back(k.lstore(k.pn(s), 1, N(k, "2")));
            in.push_back(k.chk(k.eq_c(k.add(k.leaf(k.pn(s), 0), k.leaf(k.pn(s), 1)), k.i32, "3")));
        }
        else
        {
            // y is read into a local nothing checks: the one error is the read of the member cell with no value
            Var* t {k.local(f, "t", k.i32)};
            const Expr* rd {k.leaf(k.pn(s), 1)};
            where = rd->loc;
            in.push_back(k.decl(t, rd));
        }
        in.push_back(k.es(k.call(fr, {k.pn(s)})));
        f->body = k.blk({k.decl(s, k.call(m, {})), k.iff(notnull(k, s, SP), k.blk(in))});
        k.done(f);
        return where;
    };
    lc("hv_struct", "malloc of a struct: both member cells are held, the block is freed",
       [=](Kit& k)
       {
           struct_life(k, false);
       });
    lc("hv_struct_member_unwritten", "`int t = s->y` with s->y never written reads a member cell with no value",
       [=](Kit& k)
       {
           k.fail("uninitialized", struct_life(k, true));
       });

    // struct In { int a; };  struct Out { int k; struct In in; int *q; };
    // void f(void) { struct Out *o = malloc(); if (o != NULL) { o->k = 1; o->in.a = 2; o->q = NULL; check(o->in.a == 2); free(o); } }
    const auto nested_life = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* In {k.strukt("In", {{"a", k.i32}})};
        StructType* Out {k.strukt("Out", {{"k", k.i32}, {"in", k.st(In)}, {"q", PT(k)}})};
        const Type OP {k.ptr(k.st(Out))};
        Function* m {heap_alloc_view(k, "malloc", k.st(Out), false)};
        Function* fr {heap_free_view(k, k.st(Out))};
        Function* f {k.fn("f", k.vd)};
        Var* o {k.local(f, "o", OP)};
        auto in_a = [&]()
        {
            return k.mem(k.mem(k.pn(o), 1), 0);
        };
        const Stmt* c {k.chk(k.eq_c(k.deref(in_a()), k.i32, wrong ? "3" : "2"))};
        f->body = k.blk(
            {k.decl(o, k.call(m, {})),
             k.iff(notnull(k, o, OP), k.blk({k.lstore(k.pn(o), 0, N(k, "1")), k.store(in_a(), N(k, "2")),
                                             k.lstore(k.pn(o), 2, k.nul(PT(k))), c, k.es(k.call(fr, {k.pn(o)}))}))});
        k.done(f);
        return c->loc;
    };
    lc("hv_nested",
       "malloc of a struct with a nested struct and a pointer member: every leaf cell of the block is held",
       [=](Kit& k)
       {
           nested_life(k, false);
       });
    lc("hv_nested_wrong", "check(o->in.a == 3) after writing 2 does not hold",
       [=](Kit& k)
       {
           k.fail("check", nested_life(k, true));
       });

    // calloc: every leaf is zero and has a value.
    // int *p = calloc(); if (p != NULL) { check(*p == [0|1]); free(p); }
    const auto calloc_int = [=](Kit& k, bool wrong) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "calloc", k.i32, true)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        const Stmt* c {k.chk(k.eq_c(k.deref(k.pn(p)), k.i32, wrong ? "1" : "0"))};
        f->body =
            k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk({c, k.es(k.call(fr, {k.pn(p)}))}))});
        k.done(f);
        return c->loc;
    };
    lc("hv_calloc_int", "calloc's block holds 0 and a value: check(*p == 0) holds with no write",
       [=](Kit& k)
       {
           calloc_int(k, false);
       });
    lc("hv_calloc_int_wrong", "check(*p == 1) on a calloc'd int does not hold",
       [=](Kit& k)
       {
           k.fail("check", calloc_int(k, true));
       });

    // enum color { 0..2 };  struct Z { int n; bool b; int *q; enum color c; };   mode 0: every leaf is zero; 1: check(z->b) fails
    const auto calloc_struct = [=](Kit& k, bool wrong) -> SrcLoc
    {
        const Type color {k.en("color", 32, true, "0", "2")};
        StructType* Z {k.strukt("Z", {{"n", k.i32}, {"b", k.bl}, {"q", PT(k)}, {"c", color}})};
        const Type ZP {k.ptr(k.st(Z))};
        Function* m {heap_alloc_view(k, "calloc", k.st(Z), true)};
        Function* fr {heap_free_view(k, k.st(Z))};
        Function* f {k.fn("f", k.vd)};
        Var* z {k.local(f, "z", ZP)};
        const Expr* all {
            k.land(k.land(k.eq_c(k.leaf(k.pn(z), 0), k.i32, "0"), k.lnot(k.leaf(k.pn(z), 1))),
                   k.land(k.eq(k.leaf(k.pn(z), 2), k.nul(PT(k))), k.eq(k.leaf(k.pn(z), 3), k.num(color, "0"))))};
        const Stmt* c {k.chk(wrong ? k.leaf(k.pn(z), 1) : all)};
        f->body = k.blk({k.decl(z, k.call(m, {})), k.iff(notnull(k, z, ZP), k.blk({c, k.es(k.call(fr, {k.pn(z)}))}))});
        k.done(f);
        return c->loc;
    };
    lc("hv_calloc_struct", "calloc of a struct with an int, a bool, a pointer and an enumeration: 0, false, NULL and 0",
       [=](Kit& k)
       {
           calloc_struct(k, false);
       });
    lc("hv_calloc_struct_wrong", "check(z->b) on a zeroed bool does not hold",
       [=](Kit& k)
       {
           k.fail("check", calloc_struct(k, true));
       });

    // ======== free of what is not a whole allocation ========
    // mode 0: a cell (`int x; free(&x)`); 1: a global; 2: a member of an allocation (`free(&s->y)`); 3: a borrowed parameter; 4: a callee kept a fraction of the
    // liveness; 5: a callee took the cell.   Each is invalid-free at the call (item 6: no token, or liveness not whole, or a cell not held).
    const auto free_kind = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {nullptr};
        const Expr* bad {nullptr};
        if (mode == 0)
        {
            f = k.fn("f", k.vd);
            Var* x {k.cell(f, "x", k.i32)};
            bad = k.call(fr, {k.addr(x)});
            f->body = k.blkc({k.decl(x, N(k, "0")), k.es(bad)});
        }
        else if (mode == 1)
        {
            Global* g {k.prog.add_global("g", k.i32, k.at(), nullptr)};
            g->init = N(k, "3");
            g->value = "3";
            f = k.fn("f", k.vd);
            bad = k.call(fr, {k.b.global_addr(g, k.at())});
            f->body = k.blk({k.es(bad)});
        }
        else if (mode == 2)
        {
            StructType* S {k.strukt("S", {{"x", k.i32}, {"y", k.i32}})};
            Function* m {heap_alloc_view(k, "malloc", k.st(S), false)};
            f = k.fn("f", k.vd);
            Var* s {k.local(f, "s", k.ptr(k.st(S)))};
            bad = k.call(fr, {k.mem(k.pn(s), 1)});
            f->body = k.blk({k.decl(s, k.call(m, {})), k.iff(notnull(k, s, k.ptr(k.st(S))), k.blk({k.es(bad)}))});
        }
        else if (mode == 3)
        {
            f = k.fn("f", k.vd, {{"p", PT(k)}});
            k.req(f, owns(k, FP(k, f, 0), 0));
            k.ens(f, owns(k, FP(k, f, 0), 0));
            bad = k.call(fr, {k.pn(f->param_var(0))});
            f->body = k.blk({k.es(bad)});
        }
        else
        {
            Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
            Function* other {k.fn(mode == 4 ? "keep" : "give", k.vd, {{"p", PT(k)}})};
            // takes a fraction of the liveness, or the cell, and returns nothing
            k.req(other, mode == 4 ? live(k, FP(k, other, 0), 0) : owns(k, FP(k, other, 0), 0));
            k.trusted(other);
            f = k.fn("f", k.vd);
            Var* p {k.local(f, "p", PT(k))};
            bad = k.call(fr, {k.pn(p)});
            f->body = k.blk({k.decl(p, k.call(m, {})),
                             k.iff(notnull(k, p, PT(k)),
                                   k.blk({k.store(k.pn(p), N(k, "1")), k.es(k.call(other, {k.pn(p)})), k.es(bad)}))});
        }
        k.done(f);
        return bad->loc;
    };
    // a free of an int * has four rows at the call (the lowering decides): 0 the token, 1 the layout, 2 the whole liveness, 3 the cell
    lc("hf_local", "free(&x) of a local's cell: the cell has no free token (invalid-free, row 0)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", free_kind(k, 0), 4U, 0U);
       });
    lc("hf_global", "free(&g) of a global: no token (invalid-free, row 0)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", free_kind(k, 1), 4U, 0U);
       });
    lc("hf_member", "free(&s->y) of a member of an allocation: the member's block has no token (invalid-free, row 0)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", free_kind(k, 2), 4U, 0U);
       });
    lc("hf_borrowed",
       "free(p) of a parameter the function only borrows (owns(*p), no block): no token (invalid-free, row 0)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", free_kind(k, 3), 4U, 0U);
       });
    lc("hf_keeper",
       "keep(p) takes a fraction of p's liveness and never returns it: the liveness is not whole, free fails (invalid-free, row 2)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", free_kind(k, 4), 4U, 2U);
       });
    lc("hf_cell_given", "give(p) takes the cell *p: free finds the liveness not whole (invalid-free, row 2)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", free_kind(k, 5), 4U, 2U);
       });

    // ======== leaks ========
    // void f(int c) { int *p = malloc(); [mode] }
    //   0: if (p != NULL) free(p);        verifies
    //   1: nothing                        leak at the closing brace
    //   2: if (c != 0 && p != NULL) free(p)   leak on the other path
    //   3: p = NULL (overwritten)         leak (the block is still held, nobody points to it)
    //   4: if (p != NULL) { if (c != 0) return; free(p); }    leak at the early return
    const auto leaks = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd, {{"c", k.i32}})};
        Var* p {k.local(f, "p", PT(k))};
        std::vector<const Stmt*> body {k.decl(p, k.call(m, {}))};
        if (mode == 0)
        {
            body.push_back(k.iff(notnull(k, p, PT(k)), k.blk({k.es(k.call(fr, {k.pn(p)}))})));
        }
        else if (mode == 2)
        {
            body.push_back(
                k.iff(k.land(k.ne(k.pv(f, 0), N(k, "0")), notnull(k, p, PT(k))), k.blk({k.es(k.call(fr, {k.pn(p)}))})));
        }
        else if (mode == 3)
        {
            body.push_back(k.asg(p, k.nul(PT(k))));
        }
        else if (mode == 4)
        {
            const Stmt* r {k.b.return_stmt(nullptr, k.at())};
            body.push_back(k.iff(notnull(k, p, PT(k)),
                                 k.blk({k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({r})), k.es(k.call(fr, {k.pn(p)}))})));
        }
        f->body = k.blk(body);
        k.done(f);
        return f->end_loc; // a leak is reported at the closing brace, whichever exit it is on
    };
    lc("hl_freed", "if (p != NULL) free(p): every path gives the block back, nothing is held at the end",
       [=](Kit& k)
       {
           leaks(k, 0);
       });
    lc("hl_none_freed", "malloc and never free: the function returns holding a block (leak at the closing brace)",
       [=](Kit& k)
       {
           k.fail("leak", leaks(k, 1));
       });
    lc("hl_one_path", "free only when c != 0: the other path leaks",
       [=](Kit& k)
       {
           k.fail("leak", leaks(k, 2));
       });
    lc("hl_overwritten", "p = NULL after malloc: the block is still held and nothing points to it (leak)",
       [=](Kit& k)
       {
           k.fail("leak", leaks(k, 3));
       });
    lc("hl_early_return", "a return before the free leaks the block (leak, reported at the closing brace)",
       [=](Kit& k)
       {
           k.fail("leak", leaks(k, 4));
       });

    // A return inside a loop that holds no memory but sits beside a held block: int p = malloc(); [free before the loop]; while (i < 3) { if (i == 1) return; i++; } [free]
    //   mode 0: the block is freed before the loop (verifies); 1: freed after it, so the return in the loop leaks it
    const auto leak_in_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* r {k.b.return_stmt(nullptr, k.at())};
        const Stmt* lp {
            k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                  k.blk({k.iff(k.eq_c(k.v(i), k.i32, "1"), k.blk({r})), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        const Stmt* release {k.iff(notnull(k, p, PT(k)), k.blk({k.es(k.call(fr, {k.pn(p)}))}))};
        std::vector<const Stmt*> body {k.decl(p, k.call(m, {})), k.decl(i, N(k, "0"))};
        if (mode == 0)
        {
            body.push_back(release);
        }
        body.push_back(lp);
        if (mode == 1)
        {
            body.push_back(release);
        }
        f->body = k.blk(body);
        k.done(f);
        return f->end_loc;
    };
    lc("hl_loop_return_clear", "the block is freed before the loop: the return inside it holds nothing",
       [=](Kit& k)
       {
           leak_in_loop(k, 0);
       });
    lc("hl_loop_return_leak",
       "the block is freed after the loop: the return inside it leaks (leak, reported at the closing brace)",
       [=](Kit& k)
       {
           k.fail("leak", leak_in_loop(k, 1));
       });

    // ======== the loop rule's follow-up (re-audit B1, S1, S3; record 0047 item 10, amended in M4.5d2v) ========
    // A member alias: struct S { int x; int y; };  void f(void) { struct S *s = malloc(); if (s != NULL) { s->x = 0; s->y = 0; int *x = &s->y; int i = 0;
    //   while (i < 3) { *x = i; i = i + 1; } free(s); [free(s);] } }   the pointer local x designates a member of the block the loop never names
    //   mode 0 verifies; 1: a second free of s (invalid-free); 2: no free (leak at the closing brace)
    const auto member_alias = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {k.strukt("S", {{"x", k.i32}, {"y", k.i32}})};
        const Type SP {k.ptr(k.st(S))};
        Function* m {heap_alloc_view(k, "malloc", k.st(S), false)};
        Function* fr {heap_free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* s {k.local(f, "s", SP)};
        Var* x {k.local(f, "x", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))},
                              k.blk({k.store(k.pn(x), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        std::vector<const Stmt*> in {k.lstore(k.pn(s), 0, N(k, "0")), k.lstore(k.pn(s), 1, N(k, "0")),
                                     k.decl(x, k.mem(k.pn(s), 1)), k.decl(i, N(k, "0")), lp};
        SrcLoc where;
        if (mode != 2)
        {
            in.push_back(k.es(k.call(fr, {k.pn(s)})));
        }
        if (mode == 1)
        {
            const Expr* again {k.call(fr, {k.pn(s)})};
            where = again->loc;
            in.push_back(k.es(again));
        }
        f->body = k.blk({k.decl(s, k.call(m, {})), k.iff(notnull(k, s, SP), k.blk(in))});
        k.done(f);
        return mode == 2 ? f->end_loc : where;
    };
    lc("hl_member_alias",
       "a loop writes through x = &s->y and does not name s: the block s holds is kept across the loop and freed after it",
       [=](Kit& k)
       {
           member_alias(k, 0);
       });
    lc("hl_member_alias_double_free", "the same, with a second free(s) after the first: invalid-free at the second",
       [=](Kit& k)
       {
           // a struct of two ints: the token, the offset, the liveness cell and the two leaves; the token row ends the
           // path
           k.fail_only("invalid-free", member_alias(k, 1), 5U, 0U);
       });
    lc("hl_member_alias_leak", "the same, without any free(s): the block is held at the return (leak)",
       [=](Kit& k)
       {
           k.fail("leak", member_alias(k, 2));
       });

    // A parameter that owns a block, and a member alias of it: void f(struct S *s) requires(owns_block(s)) { int *x = &s->y; int i = 0; while (i < 3) { *x = i; i++; } [free(s);] }
    //   mode 0 verifies; 1: no free (leak at the closing brace)
    const auto param_member_alias = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {k.strukt("S", {{"x", k.i32}, {"y", k.i32}})};
        const Type SP {k.ptr(k.st(S))};
        Function* fr {heap_free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd, {{"s", SP}})};
        k.req(f, block(k, FP(k, f, 0)));
        Var* x {k.local(f, "x", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))},
                              k.blk({k.store(k.pn(x), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        std::vector<const Stmt*> body {k.decl(x, k.mem(k.pn(f->param_var(0)), 1)), k.decl(i, N(k, "0")), lp};
        if (mode == 0)
        {
            body.push_back(k.es(k.call(fr, {k.pn(f->param_var(0))})));
        }
        f->body = k.blk(body);
        k.done(f);
        return f->end_loc;
    };
    lc("hl_param_member_alias",
       "a parameter owns the block s points to, the loop writes through &s->y and does not name s: s is freed after the loop",
       [=](Kit& k)
       {
           param_member_alias(k, 0);
       });
    lc("hl_param_member_alias_leak",
       "the same without the free: the block the parameter owns is held at the return (leak)",
       [=](Kit& k)
       {
           k.fail("leak", param_member_alias(k, 1));
       });

    // A cursor parameter the loop reassigns beside a block the loop does not name: int f(int *t) { int *p = malloc(); if (p != NULL) { *p = 0; int i = 0;
    //   while (i < 3) { t = NULL; i++; } [free(p);] } }   the entry value of t may be anything, in particular p
    //   mode 0 verifies; 1: no free (leak at the closing brace)
    const auto cursor = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd, {{"t", PT(k)}})};
        Var* p {k.local(f, "p", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))},
                              k.blk({k.asg(f->param_var(0), k.nul(PT(k))), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        std::vector<const Stmt*> in {k.store(k.pn(p), N(k, "0")), k.decl(i, N(k, "0")), lp};
        if (mode == 0)
        {
            in.push_back(k.es(k.call(fr, {k.pn(p)})));
        }
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk(in))});
        k.done(f);
        return f->end_loc;
    };
    lc("hl_cursor_param",
       "a loop reassigns the parameter t beside the block p holds and never names: p is freed after the loop",
       [=](Kit& k)
       {
           cursor(k, 0);
       });
    lc("hl_cursor_param_leak", "the same without the free: the block is held at the return (leak)",
       [=](Kit& k)
       {
           k.fail("leak", cursor(k, 1));
       });

    // A pointer local the loop assigns, beside the block it was copied from: void f(void) { int *p = malloc(); if (p != NULL) { *p = 0; int *q = p; int i = 0;
    //   while (i < 3) { q = p; *q = i; i++; } free(p | q); } }   the loop keeps q well-formed (S3: the automatic invariant `WF(q)`), so no row after the loop
    //   fails for the pointer's form; what the loop does not tell (that q is p again) is the pinned false alarm of heap_loop_swap_false_alarm.c
    //   mode 0 verifies (free(p)); 1: free(q): the token row and the offset row fail at the call (invalid-free) and the block p holds leaks at the closing brace
    const auto reassigned = [=](Kit& k, int mode, SrcLoc* end = nullptr) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        Var* q {k.local(f, "q", PT(k))};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* lp {
            k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                  k.blk({k.asg(q, k.pn(p)), k.store(k.pn(q), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        const Expr* rel {k.call(fr, {k.pn(mode == 0 ? p : q)})};
        std::vector<const Stmt*> in {k.store(k.pn(p), N(k, "0")), k.decl(q, k.pn(p)), k.decl(i, N(k, "0")), lp,
                                     k.es(rel)};
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk(in))});
        k.done(f);
        if (end != nullptr)
        {
            *end = f->end_loc;
        }
        return rel->loc;
    };
    lc("hl_reassigned_free", "a loop assigns q = p; the block is freed through p after it",
       [=](Kit& k)
       {
           reassigned(k, 0);
       });
    lc("hl_reassigned_free_through_q",
       "the same, freeing q: the loop does not say q is p again (invalid-free at the call, token and offset rows), and the block leaks",
       [=](Kit& k)
       {
           SrcLoc end;
           const SrcLoc at {reassigned(k, 1, &end)};
           // the token row and the offset row fail; the liveness and leaf rows sit behind them
           k.fail_only("invalid-free", at, 4U, 0U);
           k.fail_only("invalid-free", at, 4U, 1U);
           k.fail("leak", end);
       });

    // ======== leaks and the exemption of a result and of a borrowed parameter ========
    // int *make(void) ensures(r == NULL || owns_block(r)) { int *p = malloc(); [return p | return NULL] }   the block is handed to the caller, not a leak
    const auto give_result = [=](Kit& k, bool drop) -> SrcLoc
    {
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* f {k.fn("make", PT(k))};
        k.ens(f, k.lor(k.eq(k.R(f), k.nul(PT(k))), block(k, k.R(f))));
        Var* p {k.local(f, "p", PT(k))};
        const Stmt* r {k.ret(drop ? k.nul(PT(k)) : k.pn(p))};
        // the unmarked ensures owns_block(r) includes the flags: the cell is written before the block is handed over
        f->body =
            k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk({k.store(k.pn(p), N(k, "0"))})), r});
        k.done(f);
        return f->end_loc;
    };
    lc("hl_result_exempt", "make() returns the block it allocated, its ensures owns_block(r): the exit is not a leak",
       [=](Kit& k)
       {
           give_result(k, false);
       });
    lc("hl_result_dropped", "make() allocates and returns NULL: the block is held at the return (leak)",
       [=](Kit& k)
       {
           k.fail("leak", give_result(k, true));
       });

    // void use(int *p) requires(owns_block(p)) ensures(owns_block(p)) { *p = 1; }   a borrowed block is returned, not leaked
    lc("hl_borrowed_exempt", "a borrowed block (the same atom in both lists) is exempt: the function returns it",
       [=](Kit& k)
       {
           Function* f {k.fn("use", k.vd, {{"p", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           k.ens(f, block(k, FP(k, f, 0)));
           f->body = k.blk({k.store(k.pn(f->param_var(0)), N(k, "1"))});
           k.done(f);
       });

    // ======== owns_block in contracts: consume, give, borrow ========
    // void eat(int *p) requires(owns_block(p)) { [free(p);] }   a consuming parameter must free its block
    const auto consume = [=](Kit& k, bool do_free) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32)};
        Function* f {k.fn("eat", k.vd, {{"p", PT(k)}})};
        k.req(f, block(k, FP(k, f, 0)));
        f->body =
            k.blk({do_free ? k.es(k.call(fr, {k.pn(f->param_var(0))})) : k.store(k.pn(f->param_var(0)), N(k, "1"))});
        k.done(f);
        return f->end_loc;
    };
    lc("ho_consume", "requires(owns_block(p)) and free(p): the function took the block and ended it",
       [=](Kit& k)
       {
           consume(k, true);
       });
    lc("ho_consume_leak", "requires(owns_block(p)) and no free: the block is held when it returns (leak)",
       [=](Kit& k)
       {
           k.fail("leak", consume(k, false));
       });

    // struct A { int bal; };  struct A *open(void) ensures(r == NULL || (owns_block(r) && r->bal == [0|1])) { s = malloc(); if (s) s->bal = 0; return s; }
    // then the caller:  struct A *a = open(); if (a != NULL) { check(a->bal == 0); [free(a);] }
    // 0 verifies; 1: the ensures promises 1 (postcondition at the return); 2: the caller does not free (leak)
    const auto give = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* A {k.strukt("A", {{"bal", k.i32}})};
        const Type AP {k.ptr(k.st(A))};
        Function* m {heap_alloc_view(k, "malloc", k.st(A), false)};
        Function* fr {heap_free_view(k, k.st(A))};
        Function* open {k.fn("open", AP)};
        k.ens(open, k.lor(k.eq(k.R(open), k.nul(AP)),
                          k.land(block(k, k.R(open)), k.eq_c(k.leaf(k.R(open), 0), k.i32, mode == 1 ? "1" : "0"))));
        Var* s {k.local(open, "s", AP)};
        const Stmt* r {k.ret(k.pn(s))};
        open->body =
            k.blk({k.decl(s, k.call(m, {})), k.iff(notnull(k, s, AP), k.blk({k.lstore(k.pn(s), 0, N(k, "0"))})), r});
        k.done(open);
        Function* f {k.fn("f", k.vd)};
        Var* a {k.local(f, "a", AP)};
        std::vector<const Stmt*> in;
        if (mode != 1)
        {
            // mode 1: the callee's ensures promises 1, a check of 0 would contradict it
            in.push_back(k.chk(k.eq_c(k.leaf(k.pn(a), 0), k.i32, "0")));
        }
        if (mode != 2)
        {
            in.push_back(k.es(k.call(fr, {k.pn(a)})));
        }
        f->body = k.blk({k.decl(a, k.call(open, {})), k.iff(notnull(k, a, AP), k.blk(in))});
        k.done(f);
        return mode == 1 ? r->loc : f->end_loc;
    };
    lc("ho_give",
       "open() returns a block with bal == 0 and the caller frees it: the postcondition reads r->bal through owns_block(r)",
       [=](Kit& k)
       {
           give(k, 0);
       });
    lc("ho_give_wrong_value", "the same promising bal == 1: the postcondition fails at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", give(k, 1));
       });
    lc("ho_give_caller_leaks", "the caller of open() never frees what it was given (leak at its closing brace)",
       [=](Kit& k)
       {
           k.fail("leak", give(k, 2));
       });

    // void bump(int *p) requires(owns_block(p)) ensures(owns_block(p)) { [*p = 1; | free(p);] }   a borrowed block is returned whole
    // caller: p = malloc(); if (p != NULL) { *p = 0; bump(p); free(p); }
    const auto borrow = [=](Kit& k, bool callee_frees) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32)};
        Function* bump {k.fn("bump", k.vd, {{"p", PT(k)}})};
        k.req(bump, block(k, FP(k, bump, 0)));
        k.ens(bump, block(k, FP(k, bump, 0)));
        bump->body = k.blk({callee_frees ? k.es(k.call(fr, {k.pn(bump->param_var(0))}))
                                         : k.store(k.pn(bump->param_var(0)), N(k, "1"))});
        k.done(bump);
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", PT(k))};
        f->body = k.blk({k.decl(p, k.call(m, {})),
                         k.iff(notnull(k, p, PT(k)), k.blk({k.store(k.pn(p), N(k, "0")), k.es(k.call(bump, {k.pn(p)})),
                                                            k.es(k.call(fr, {k.pn(p)}))}))});
        k.done(f);
        return bump->end_loc;
    };
    lc("ho_borrow", "bump borrows the block and returns it: the caller still owns it and frees it",
       [=](Kit& k)
       {
           borrow(k, false);
       });
    lc("ho_borrow_freed",
       "the callee frees what it promised to return: its ensures owns_block(p) does not hold (permission at its closing brace: five rows, row 1)",
       [=](Kit& k)
       {
           k.fail_only("permission", borrow(k, true), 5U, 1U);
       });

    // the caller of a consuming function may not use the block afterwards, nor pass one it only borrows
    //   void eat(int *p) requires(owns_block(p)) [trusted]
    //   mode 0: p = malloc(); if (p) { *p = 0; eat(p); }   verifies;  1: ... eat(p); *p = 1;  (permission at the store)
    //   2: void g(int *q) requires(owns_block(q)) ensures(owns_block(q)) { eat(q); }  (permission at the call: the block is gone on return)
    const auto eat_caller = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* eat {k.fn("eat", k.vd, {{"p", PT(k)}})};
        k.req(eat, block(k, FP(k, eat, 0)));
        k.trusted(eat);
        Function* f {nullptr};
        SrcLoc where;
        if (mode == 2)
        {
            f = k.fn("g", k.vd, {{"q", PT(k)}});
            k.req(f, block(k, FP(k, f, 0)));
            k.ens(f, block(k, FP(k, f, 0)));
            f->body = k.blk({k.es(k.call(eat, {k.pn(f->param_var(0))}))});
            k.done(f);
            return f->end_loc;
        }
        Function* m {heap_alloc_view(k, "malloc", k.i32, false)};
        f = k.fn("f", k.vd);
        Var* p {k.local(f, "p", PT(k))};
        std::vector<const Stmt*> in {k.store(k.pn(p), N(k, "0")), k.es(k.call(eat, {k.pn(p)}))};
        if (mode == 1)
        {
            const Stmt* w {k.store(k.pn(p), N(k, "1"))};
            where = w->loc;
            in.push_back(w);
        }
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(notnull(k, p, PT(k)), k.blk(in))});
        k.done(f);
        return where;
    };
    lc("ho_eat_caller", "eat(p) takes the block: nothing is used or held afterwards",
       [=](Kit& k)
       {
           eat_caller(k, 0);
       });
    lc("ho_eat_use_after", "*p = 1 after eat(p) took the block (permission at the store)",
       [=](Kit& k)
       {
           k.fail("permission", eat_caller(k, 1));
       });
    lc("ho_eat_borrowed",
       "g borrows q (owns_block in both lists) and passes it to eat: g returns without the block it promised (permission at its closing brace: five rows, row 1)",
       [=](Kit& k)
       {
           k.fail_only("permission", eat_caller(k, 2), 5U, 1U);
       });

    // S4: the returns' separation rows.  int *f(int *p) requires(owns_block(p)) ensures(owns_block(p)) ensures(r == NULL || owns_block(r)) { return p; }
    // p is borrowed and handed out as the result: the two atoms would name one block (a row at the return, never Silicon's own postcondition check)
    lc("ho_return_separation",
       "returning the borrowed block as the result: owns_block(p) and owns_block(r) would name one block (permission at the return)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", PT(k), {{"p", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           k.ens(f, block(k, FP(k, f, 0)));
           k.ens(f, k.lor(k.eq(k.R(f), k.nul(PT(k))), block(k, k.R(f))));
           const Stmt* r {k.ret(k.pn(f->param_var(0)))};
           f->body = k.blk({r});
           k.done(f);
           k.fail_only("permission", r->loc, 11U, 0U);
       });

    // ======== sequencing: free(p) beside another use of p (item 9) ========
    //   int f(int *p, int *q) requires(owns_block(p) && owns_block(q)) { return (free(p), 0) + use_live(X); }   X = q verifies
    //   mode 0: use_live(q); 1: use_live(p) (unsequenced); 2: use_live(p) first, then the free (unsequenced); 3: (p != q); 4: *p; 5: *p first, then the free
    const auto beside_free = [=](Kit& k, int mode) -> SrcLoc
    {
        Function* fr {heap_free_view(k, k.i32)};
        Function* ul {k.fn("use_live", k.i32, {{"x", PT(k)}})};
        k.req(ul, live(k, FP(k, ul, 0), 0));
        k.ens(ul, live(k, FP(k, ul, 0), 0));
        k.trusted(ul);
        Function* f {k.fn("f", k.i32, {{"p", PT(k)}, {"q", PT(k)}})};
        k.req(f, k.land(block(k, FP(k, f, 0)), block(k, FP(k, f, 1))));
        k.ens(f, block(k, FP(k, f, 1))); // q is borrowed and returned
        const Var* p {f->param_var(0)};
        const Var* q {f->param_var(1)};
        const Expr* freed {k.comma(k.call(fr, {k.pn(p)}), N(k, "0"))};
        const Expr* other {nullptr};
        const Expr* flagged {nullptr}; // the node the unsequenced report names (the later-created access or call)
        if (mode == 0)
        {
            other = k.call(ul, {k.pn(q)});
        }
        else if (mode == 1 || mode == 2)
        {
            const Expr* pa {k.pn(p)}; // the report names the use of p: the argument node
            other = k.call(ul, {pa});
            flagged = pa;
        }
        else if (mode == 3)
        {
            const Expr* pa {k.pn(p)};
            other = k.bi(k.i32, k.ne(pa, k.pn(q)));
            flagged = pa;
        }
        else
        {
            other = k.deref(k.pn(p));
            flagged = other;
        }
        const Expr* sum {(mode == 2 || mode == 5) ? k.add(other, freed) : k.add(freed, other)};
        const Stmt* r {k.ret(sum)};
        f->body = k.blk({r});
        k.done(f);
        return flagged != nullptr ? flagged->loc : SrcLoc {};
    };
    lc("hs_free_beside_other_block",
       "(free(p), 0) + use_live(q) with q another block: no shared object, nothing unsequenced",
       [=](Kit& k)
       {
           beside_free(k, 0);
       });
    lc("hs_free_then_use_live",
       "(free(p), 0) + use_live(p): the free ends p's lifetime and the call uses it, in no order (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", beside_free(k, 1));
       });
    lc("hs_use_live_then_free", "use_live(p) + (free(p), 0): the same in the other order (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", beside_free(k, 2));
       });
    lc("hs_free_beside_compare", "(free(p), 0) + (p != q): the comparison uses p's liveness (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", beside_free(k, 3));
       });
    lc("hs_free_beside_deref", "(free(p), 0) + *p: the read of the cell beside the free that takes it (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", beside_free(k, 4));
       });
    lc("hs_deref_beside_free", "*p + (free(p), 0): the same in the other order (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", beside_free(k, 5));
       });
}

// ---- check_program's backstops for the owns_block atom and the two VIR flags (decision 0047 items 2, 4, 5; the encoder reports each first) ----
void define_heap_rejections()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto PPT = [](Kit& k)
    {
        return k.ptr(k.ptr(k.i32));
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

    // an owns_block atom in a loop invariant (M4.5d2b)
    rc("hb_rej_invariant", "an `owns_block` atom in a loop invariant",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           Var* q {k.local(f, "q", PT(k))};
           Var* i {k.local(f, "i", k.i32)};
           f->body = k.blk({k.decl(q, k.pn(f->param_var(0))), k.decl(i, N(k, "1")),
                            k.whl(k.gt(k.v(i), N(k, "0")), {k.cl(block(k, k.pn(q)))}, {k.cl(k.v(i))},
                                  k.blk({k.asg(i, k.sub(k.v(i), N(k, "1")))}))});
           k.done(f);
       });
    // on something that is not a pointer parameter or the result: a load (`*pp`)
    rc("hb_rej_load", "something other than a pointer parameter or",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"pp", PPT(k)}})};
           k.req(f, block(k, k.deref(FP(k, f, 0))));
           f->body = k.blk({});
           k.done(f);
       });
    // on a global
    rc("hb_rej_global", "something other than a pointer parameter or",
       [=](Kit& k)
       {
           Global* g {k.prog.add_global("g", k.i32, k.at(), nullptr)};
           g->init = N(k, "0");
           g->value = "0";
           Function* f {k.fn("f", k.vd)};
           k.req(f, block(k, k.b.global_addr(g, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    // the result in a requires (it has no value there)
    rc("hb_rej_result_in_requires", "the result outside an `ensures`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", PT(k))};
           k.req(f, block(k, k.R(f)));
           f->body = k.blk({k.ret(k.nul(PT(k)))});
           k.done(f);
       });
    // in a measure or in code
    rc("hb_rej_in_code", "a permission atom outside a contract",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           f->body = k.blk({k.chk(block(k, k.pn(f->param_var(0))))});
           k.done(f);
       });
    // under old
    rc("hb_rej_under_old", "a permission atom under `old`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           k.ens(f, k.old(block(k, FP(k, f, 0))));
           f->body = k.blk({});
           k.done(f);
       });
    // a ghost amount or the implicit flag
    rc("hb_rej_amount", "a ghost amount or marked implicit",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, FP(k, f, 0), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    // a pointer to void: VIR never holds void*
    rc("hb_rej_void_pointer", "pointer to void",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.vd)}})};
           k.req(f, block(k, FP(k, f, 0)));
           f->body = k.blk({});
           k.done(f);
       });
    // twice in one list
    rc("hb_rej_twice", "the same `owns_block` atom twice",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, k.land(block(k, FP(k, f, 0)), block(k, FP(k, f, 0))));
           f->body = k.blk({});
           k.done(f);
       });
    // beside owns / reads / live of the same pointer
    rc("hb_rej_beside_owns", "beside `owns`, `reads` or `live`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, k.land(block(k, FP(k, f, 0)), k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("hb_rej_beside_live", "beside `owns`, `reads` or `live`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, k.land(block(k, FP(k, f, 0)), k.b.atom(AtomKind::Live, FP(k, f, 0), 0, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    // an ensures atom on a formal with no requires partner in a function without a body (decision 0052 item 2: with a
    // body the callee may give a block it holds)
    rc("hb_rej_ensures_without_requires", "without one in the `requires`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.ens(f, block(k, FP(k, f, 0)));
           k.trusted(f);
           k.done(f);
       });
    // result_init: a pointer result and an ensures owns_block of the result
    rc("hb_rej_result_init_no_atom", "without `owns_block` of the result",
       [=](Kit& k)
       {
           Function* f {k.fn("f", PT(k))};
           f->result_init = InitMarker::RefToUninit;
           f->body = k.blk({k.ret(k.nul(PT(k)))});
           k.done(f);
       });
    rc("hb_rej_result_init_not_pointer", "does not return a pointer",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           f->result_init = InitMarker::RefToUninit;
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("hb_rej_result_init_must_init", "not `ref_to_uninit`",
       [=](Kit& k)
       {
           Function* f {k.fn("f", PT(k))};
           k.ens(f, k.lor(k.eq(k.R(f), k.nul(PT(k))), block(k, k.R(f))));
           f->result_init = InitMarker::MustInit;
           f->body = k.blk({k.ret(k.nul(PT(k)))});
           k.done(f);
       });
    // deallocates: a void function of one pointer parameter, a requires owns_block of it, no ensures owns_block of it
    rc("hb_rej_deallocates_result", "is not a void function",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"p", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           f->deallocates = true;
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("hb_rej_deallocates_two_params", "is not a void function of exactly one pointer parameter",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}, {"q", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           f->deallocates = true;
           f->body = k.blk({});
           k.done(f);
       });
    rc("hb_rej_deallocates_no_requires", "does not take `owns_block` of its parameter",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           f->deallocates = true;
           f->body = k.blk({});
           k.done(f);
       });
    rc("hb_rej_deallocates_returns_it", "or its `ensures` returns it",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PT(k)}})};
           k.req(f, block(k, FP(k, f, 0)));
           k.ens(f, block(k, FP(k, f, 0)));
           f->deallocates = true;
           f->body = k.blk({});
           k.done(f);
       });
}

} // namespace
