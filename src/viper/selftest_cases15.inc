// (included by selftest.cpp) Struct values (decision 0044, M4.5c2a): a by-value struct parameter is the tuple of its members (item 3: each member a formal, the copy a struct block
// written from the formals at entry, `s.x` in a contract the entry value), a by-value result is one Viper result per member (item 4: `return` reads every member, the contract
// reads `r.x`), a struct-valued call is a multi-target call whose targets are stored into the destination's leaves (item 5).  These cases replace the rejection cases
// `struct_param_by_value` and `struct_result_by_value` of selftest_cases14.inc (question 101: the constructs they rejected are accepted now).  PRE-REGISTERED from the record
// before lowering ran them: a postcondition fails at the `return`, a check at its statement; the backstop rejections of `check_program` keep their messages.  A struct value in VIR
// is an `Aggregate` of leaf reads (a struct object read as a whole: an argument, a return value), a `Call` of a struct-returning function, or a `Field` of one.
namespace {

void define_struct_values()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_s = [](Kit& k)
    {
        return k.strukt("S", {{"x", k.i32}, {"y", k.i32}});
    };
    // the members of the struct object `&v`, as the whole value `(struct S){v.x, v.y}` an argument or a return reads
    auto whole = [](Kit& k, const StructType* S, const Var* v)
    {
        return k.b.aggregate(k.st(S), {k.leaf(k.addr(v), 0), k.leaf(k.addr(v), 1)}, k.at());
    };
    auto field = [](Kit& k, const Expr* a, int index)
    {
        return k.b.field(a, index, k.at());
    };

    // int get(struct S s) requires(s.x > 0) ensures(r == s.x | s.y) { return s.x; }   int use(void) ensures(r == 3) { struct S s; s.x = 3; s.y = 4; return get(s); }
    const auto param = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S = mk_s(k);
        Function* get = k.fn("get", k.i32, {{"s", k.st(S)}});
        get->mark_address_taken(get->param_var(0));
        k.req(get, k.gt(field(k, k.F(get, 0), 0), N(k, "0")));
        k.ens(get, k.eq(k.R(get), field(k, k.F(get, 0), wrong ? 1 : 0)));
        const Stmt* ret = k.ret(k.leaf(k.addr(get->param_var(0)), 0));
        get->body = k.blkc({ret});
        k.done(get);
        Function* use = k.fn("use", k.i32);
        if (!wrong)
        {
            k.ens(use, k.eq_c(k.R(use), k.i32, "3"));
        }
        Var* s = k.scell(use, "s", S);
        use->body = k.blkc({k.decl(s, nullptr), k.lstore(k.addr(s), 0, N(k, "3")), k.lstore(k.addr(s), 1, N(k, "4")),
                            k.ret(k.call(get, {whole(k, S, s)}))});
        k.done(use);
        return ret;
    };
    lc("struct_value_param",
       "a by-value struct parameter: the members are the formals, s.x in the contract is the entry value, the caller passes the leaves it read",
       [=](Kit& k)
       {
           param(k, false);
       });
    lc("struct_value_param_post", "the ensures reads s.y but get returns s.x: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", param(k, true)->loc);
       });

    // struct S make(int a) ensures(r.x == a && r.y == 2) { struct S s; s.x = a; s.y = 2; return s; }
    // int use(void) ensures(r == 1) { struct S t; t = make(1); check(t.x == 1); check(t.y == 2 | 1); return t.x; }
    const auto result = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S = mk_s(k);
        Function* make = k.fn("make", k.st(S), {{"a", k.i32}});
        k.ens(make, k.land(k.eq(field(k, k.R(make), 0), k.F(make, 0)), k.eq(field(k, k.R(make), 1), N(k, "2"))));
        Var* s = k.scell(make, "s", S);
        make->body = k.blkc({k.decl(s, nullptr), k.lstore(k.addr(s), 0, k.pv(make, 0)),
                             k.lstore(k.addr(s), 1, N(k, "2")), k.ret(whole(k, S, s))});
        k.done(make);
        Function* use = k.fn("use", k.i32);
        k.ens(use, k.eq_c(k.R(use), k.i32, "1"));
        Var* t = k.scell(use, "t", S);
        const Stmt* chk = k.chk(k.eq(k.leaf(k.addr(t), 1), N(k, wrong ? "1" : "2")));
        use->body = k.blkc({k.decl(t, nullptr), k.store(k.addr(t), k.call(make, {N(k, "1")})),
                            k.chk(k.eq(k.leaf(k.addr(t), 0), N(k, "1"))), chk, k.ret(k.leaf(k.addr(t), 0))});
        k.done(use);
        return chk;
    };
    lc("struct_value_result",
       "a by-value struct result: return s reads every member, the call's results are stored into the destination's leaves, t.x == 1 and t.y == 2 follow from the ensures",
       [=](Kit& k)
       {
           result(k, false);
       });
    lc("struct_value_result_check", "check(t.y == 1) after t = make(1): the ensures says r.y == 2",
       [=](Kit& k)
       {
           k.fail("check", result(k, true)->loc);
       });

    // ---- the backstop rejections (a struct value exists only in item 2's positions) ----
    rc("struct_param_copy_not_address_taken", "address-taken",
       [=](Kit& k)
       {
           StructType* S = mk_s(k);
           Function* f = k.fn("f", k.vd, {{"s", k.st(S)}}); // the copy of a by-value parameter must be a cell (item 3)
           f->body = k.blk({});
           k.done(f);
       });
    rc("struct_value_conditional", "does not allow",
       [=](Kit& k)
       {
           StructType* S = mk_s(k);
           Function* f = k.fn("f", k.vd, {{"c", k.i32}});
           Var* t = k.scell(f, "t", S);
           const auto agg = [&](const char* a, const char* b)
           {
               return k.b.aggregate(k.st(S), {N(k, a), N(k, b)}, k.at());
           };
           f->body = k.blkc({k.decl(t, nullptr),
                             k.store(k.addr(t), k.cond(k.ne(k.pv(f, 0), N(k, "0")), agg("1", "2"),
                                                       agg("3", "4")))}); // t = c ? {1, 2} : {3, 4}: a struct ?: (M8)
           k.done(f);
       });
}

void define_struct_literals()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_s = [](Kit& k)
    {
        return k.strukt("S", {{"x", k.i32}, {"y", k.i32}});
    };
    // struct S *p = &(struct S){2, 3}; check(p->x == 2 | 9); return p->x + p->y;      int f(void) ensures(r == 5)
    const auto lit = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S = mk_s(k);
        Function* f = k.fn("f", k.i32);
        k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
        Var* cl = k.scell(f, "(struct S){2, 3}", S);
        cl->compound_literal = true;
        Var* p = k.local(f, "p", k.ptr(k.st(S)));
        const Expr* store = k.b.store_expr(k.addr(cl), k.b.aggregate(k.st(S), {N(k, "2"), N(k, "3")}, k.at()), k.at());
        const Expr* value = k.b.comma(store, k.addr(cl), k.at());
        const Stmt* chk = k.chk(k.eq(k.leaf(k.pn(p), 0), N(k, wrong ? "9" : "2")));
        f->body =
            k.blkc({k.decl(cl, nullptr), k.decl(p, value), chk, k.ret(k.add(k.leaf(k.pn(p), 0), k.leaf(k.pn(p), 1)))});
        k.done(f);
        return chk;
    };
    lc("struct_literal",
       "&(struct S){2, 3}: the hidden object is allocated at block entry, every element evaluated, then every leaf stored; p->x + p->y is 5",
       [=](Kit& k)
       {
           lit(k, false);
       });
    lc("struct_literal_check", "check(p->x == 9) after p = &(struct S){2, 3}: check fails",
       [=](Kit& k)
       {
           k.fail("check", lit(k, true)->loc);
       });
    // the swap: s = *&(struct S){s.y, s.x} style: elements read the destination of another object, all read before any leaf is written
    // struct S t = {1, 2}; struct S *p = &(struct S){t.y, t.x}; check(p->x == 2 && p->y == 1);
    lc("struct_literal_elements_first",
       "&(struct S){t.y, t.x}: both elements are read before the first leaf of the literal is written (audit B1)",
       [=](Kit& k)
       {
           StructType* S = mk_s(k);
           Function* f = k.fn("f", k.vd);
           Var* t = k.scell(f, "t", S);
           Var* cl = k.scell(f, "(struct S){t.y, t.x}", S);
           cl->compound_literal = true;
           Var* p = k.local(f, "p", k.ptr(k.st(S)));
           const Expr* agg = k.b.aggregate(k.st(S), {k.leaf(k.addr(t), 1), k.leaf(k.addr(t), 0)}, k.at());
           const Expr* value = k.b.comma(k.b.store_expr(k.addr(cl), agg, k.at()), k.addr(cl), k.at());
           f->body = k.blkc({k.decl(t, nullptr), k.lstore(k.addr(t), 0, N(k, "1")), k.lstore(k.addr(t), 1, N(k, "2")),
                             k.decl(cl, nullptr), k.decl(p, value),
                             k.chk(k.land(k.eq(k.leaf(k.pn(p), 0), N(k, "2")), k.eq(k.leaf(k.pn(p), 1), N(k, "1"))))});
           k.done(f);
       });
    // int *p = &(int){5}; *p += 1; check(*p == 6 | 7);
    const auto scalar = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f = k.fn("f", k.vd);
        Var* cl = k.cell(f, "(int){5}", k.i32);
        cl->compound_literal = true;
        Var* p = k.local(f, "p", k.ptr(k.i32));
        const Expr* value = k.b.comma(k.b.store_expr(k.addr(cl), N(k, "5"), k.at()), k.addr(cl), k.at());
        const Stmt* chk = k.chk(k.eq(k.deref(k.pn(p)), N(k, wrong ? "7" : "6")));
        f->body = k.blkc({k.decl(cl, nullptr), k.decl(p, value), k.cstore(k.pn(p), BinaryOp::Add, N(k, "1")), chk});
        k.done(f);
        return chk;
    };
    lc("scalar_literal", "&(int){5}: a scalar literal is the store of its converted element into the hidden cell",
       [=](Kit& k)
       {
           scalar(k, false);
       });
    lc("scalar_literal_check", "check(*p == 7) after *p += 1 on &(int){5}: check fails",
       [=](Kit& k)
       {
           k.fail("check", scalar(k, true)->loc);
       });
    // a literal in a loop body: the hidden object is allocated at every iteration's block entry and released at its end
    // int s = 0; int i = 0; while (i < 3) { struct S *q = &(struct S){i, 1}; s += q->y; i += 1; }  check(s == 3 | 4)
    const auto loop = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S = mk_s(k);
        Function* f = k.fn("f", k.vd);
        Var* s = k.local(f, "s", k.i32);
        Var* i = k.local(f, "i", k.i32);
        Var* cl = k.scell(f, "(struct S){i, 1}", S);
        cl->compound_literal = true;
        Var* q = k.local(f, "q", k.ptr(k.st(S)));
        const Expr* agg = k.b.aggregate(k.st(S), {k.pn(i), N(k, "1")}, k.at());
        const Expr* value = k.b.comma(k.b.store_expr(k.addr(cl), agg, k.at()), k.addr(cl), k.at());
        const Stmt* body =
            k.blkc({k.decl(cl, nullptr), k.decl(q, value), k.b.assign(s, k.add(k.pn(s), k.leaf(k.pn(q), 1)), k.at()),
                    k.b.assign(i, k.add(k.pn(i), N(k, "1")), k.at())});
        const Stmt* w = k.b.while_stmt(
            k.lt(k.pn(i), N(k, "3")),
            {k.cl(k.land(k.le(N(k, "0"), k.pn(i)), k.land(k.le(k.pn(i), N(k, "3")), k.eq(k.pn(s), k.pn(i)))))},
            {k.cl(k.sub(N(k, "3"), k.pn(i)))}, body, k.at());
        const Stmt* chk = k.chk(k.eq(k.pn(s), N(k, wrong ? "4" : "3")));
        f->body = k.blkc({k.decl(s, N(k, "0")), k.decl(i, N(k, "0")), w, chk});
        k.done(f);
        return chk;
    };
    lc("struct_literal_loop",
       "a literal in a loop body: allocated at each iteration's block entry, released at its end",
       [=](Kit& k)
       {
           loop(k, false);
       });
    lc("struct_literal_loop_check", "check(s == 4) after three iterations adding 1",
       [=](Kit& k)
       {
           k.fail("check", loop(k, true)->loc);
       });
    // use after the block: int *q; { q = &(int){5}; } return *q;  -- `lifetime` at the closing brace of the inner block is the scope exit; the use is the error
    lc("struct_literal_lifetime", "a pointer to the literal used after its block: the use is a lifetime error",
       [=](Kit& k)
       {
           StructType* S = mk_s(k);
           Function* f = k.fn("f", k.i32);
           Var* p = k.local(f, "p", k.ptr(k.st(S)));
           Var* cl = k.scell(f, "(struct S){2, 3}", S);
           cl->compound_literal = true;
           const Expr* agg = k.b.aggregate(k.st(S), {N(k, "2"), N(k, "3")}, k.at());
           const Expr* value = k.b.comma(k.b.store_expr(k.addr(cl), agg, k.at()), k.addr(cl), k.at());
           const Stmt* inner = k.blkc({k.decl(cl, nullptr), k.b.assign(p, value, k.at())});
           const Expr* rd = k.leaf(k.pn(p), 0);
           const Stmt* use = k.ret(rd);
           f->body = k.blkc({k.decl(p, k.nul(k.ptr(k.st(S)))), inner, use});
           k.done(f);
           k.fail("permission", rd->loc);
       });
}

// ---- struct globals and static struct locals (decision 0044 items 8 and 9): program_start's row per leaf, the facts with the member lengths, owns(origin) per leaf ----
void define_struct_globals()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_s = [](Kit& k)
    {
        return k.strukt("S", {{"x", k.i32}, {"y", k.i32}});
    };
    // a struct global of type S with the initial values of its two leaves (an owner: a static local of that function)
    auto glob_s = [](Kit& k, StructType* S, const char* name, const char* vx, const char* vy,
                     const Function* owner = nullptr) -> Global*
    {
        Global* g = k.prog.add_global(name, k.st(S), k.at(), owner);
        for (const char* v : {vx, vy})
        {
            GlobalLeaf l;
            l.kind = Global::Init::Value;
            l.init = k.num(k.i32, v);
            l.value = v;
            l.loc = k.at();
            g->leaves.push_back(l);
        }
        return g;
    };
    auto ga = [](Kit& k, const Global* g)
    {
        return k.b.global_addr(g, k.at());
    };
    auto rd = [=](Kit& k, const Global* g, int m)
    {
        return k.leaf(ga(k, g), m);
    }; // a read of g.m
    auto gatom = [=](Kit& k, AtomKind kind, const Expr* place, bool implicit = false)
    {
        return k.b.global_atom(kind, place, 0, k.at(), implicit);
    };

    // static struct S origin; int main() { check(origin.x == 0 && origin.y == 0); origin.x = 5; check(origin.x == 5 | 6 && origin.y == 0); }
    const auto start = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S = mk_s(k);
        Global* g = glob_s(k, S, "origin", "0", "0");
        Function* m = k.fn("main", k.i32);
        const Stmt* c1 = k.chk(k.land(k.eq_c(rd(k, g, 0), k.i32, "0"), k.eq_c(rd(k, g, 1), k.i32, "0")));
        const Stmt* w = k.lstore(ga(k, g), 0, N(k, "5"));
        const Stmt* c2 = k.chk(k.land(k.eq_c(rd(k, g, 0), k.i32, wrong ? "6" : "5"), k.eq_c(rd(k, g, 1), k.i32, "0")));
        m->body = k.blk({c1, w, c2, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c2;
    };
    lc("sglob_start_main",
       "a struct global is held by main from program_start, every leaf with its initial value (a row per leaf, one liveness fraction); main writes one leaf and keeps the other",
       [=](Kit& k)
       {
           start(k, false);
       });
    lc("sglob_start_main_check", "after origin.x = 5 the check origin.x == 6 fails",
       [=](Kit& k)
       {
           k.fail("check", start(k, true)->loc);
       });

    // void bump() requires(owns(origin) && origin.x < 100) ensures(owns(origin) && origin.x == old(origin.x) + 1 && origin.y == old(origin.y)) { origin.x = origin.x + 1; }
    // int main() { bump(); check(origin.x == 1 | 2 && origin.y == 0); }
    const auto owns_call = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S = mk_s(k);
        Global* g = glob_s(k, S, "origin", "0", "0");
        Function* f = k.fn("bump", k.vd);
        k.req(f, k.land(gatom(k, AtomKind::Owns, ga(k, g)), k.lt(rd(k, g, 0), N(k, "100"))));
        k.ens(f,
              k.land(gatom(k, AtomKind::Owns, ga(k, g)), k.land(k.eq(rd(k, g, 0), k.add(k.old(rd(k, g, 0)), N(k, "1"))),
                                                                k.eq(rd(k, g, 1), k.old(rd(k, g, 1))))));
        f->body = k.blk({k.lstore(ga(k, g), 0, k.add(rd(k, g, 0), N(k, "1")))});
        k.done(f);
        Function* m = k.fn("main", k.i32);
        const Stmt* chk = k.chk(k.land(k.eq_c(rd(k, g, 0), k.i32, wrong ? "2" : "1"), k.eq_c(rd(k, g, 1), k.i32, "0")));
        m->body = k.blk({k.es(k.call(f, {})), chk, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return chk;
    };
    lc("sglob_owns_call",
       "owns(origin) is every leaf and one liveness fraction: bump's ensures come back with old(origin.x) and the frame of origin.y",
       [=](Kit& k)
       {
           owns_call(k, false);
       });
    lc("sglob_owns_call_check", "after bump() the check origin.x == 2 fails",
       [=](Kit& k)
       {
           k.fail("check", owns_call(k, true)->loc);
       });

    // void touch() requires(owns(origin.x)) ensures(owns(origin.x)) { origin.y = 1; }
    lc("sglob_member_unowned", "owns(origin.x) is one leaf: the write to origin.y has no permission",
       [=](Kit& k)
       {
           StructType* S = mk_s(k);
           Global* g = glob_s(k, S, "origin", "0", "0");
           Function* f = k.fn("touch", k.vd);
           k.req(f, gatom(k, AtomKind::Owns, k.mem(ga(k, g), 0)));
           k.ens(f, gatom(k, AtomKind::Owns, k.mem(ga(k, g), 0)));
           const Stmt* w = k.lstore(ga(k, g), 1, N(k, "1"));
           f->body = k.blk({w});
           k.done(f);
           k.fail("permission", w->loc);
       });

    // void f() { static struct S c; if (c.x < 100) c.x++; }   void r(int n) requires(0 <= n <= 10) decreases(n) { if (n > 0) { f(); r(n - 1); } }   int main() { f(); r(3); }
    // mode 0 verifies, 1: r without the implicit atom (permission at its call of f)
    const auto statics = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S = mk_s(k);
        Function* f = k.fn("f", k.vd);
        Global* c = glob_s(k, S, "c", "0", "0", f);
        k.req(f, gatom(k, AtomKind::Owns, ga(k, c), true));
        k.ens(f, gatom(k, AtomKind::Owns, ga(k, c), true));
        f->body = k.blk(
            {k.iff(k.lt(rd(k, c, 0), N(k, "100")), k.blk({k.lstore(ga(k, c), 0, k.add(rd(k, c, 0), N(k, "1")))}))});
        k.done(f);
        Function* r = k.fn("r", k.vd, {{"n", k.i32}});
        k.req(r, k.range(k.F(r, 0), k.i32, "0", "10"));
        if (mode == 0)
        {
            k.req(r, gatom(k, AtomKind::Owns, ga(k, c), true));
            k.ens(r, gatom(k, AtomKind::Owns, ga(k, c), true));
        }
        k.dec(r, k.F(r, 0));
        const Expr* cf = k.call(f, {});
        r->body = k.blk(
            {k.iff(k.gt(k.pv(r, 0), N(k, "0")), k.blk({k.es(cf), k.es(k.call(r, {k.sub(k.pv(r, 0), N(k, "1"))}))}))});
        k.done(r);
        Function* m = k.fn("main", k.i32);
        m->body = k.blk({k.es(k.call(f, {})), k.es(k.call(r, {N(k, "3")})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return cf->loc;
    };
    lc("sglob_static_recursion",
       "the static struct local c of f is held by f and by r (which calls f and itself) as an implicit owns of every leaf, and by main from program_start",
       [=](Kit& k)
       {
           statics(k, 0);
       });
    lc("sglob_static_unheld", "r without the implicit atom holds no permission to c: permission at its call of f",
       [=](Kit& k)
       {
           k.fail_only("permission", statics(k, 1), 13, 3);
       });

    // a nested struct global: struct T { struct S s; int z; } t; main reads t.s.y and writes t.z
    lc("sglob_nested",
       "a nested struct global: the slot facts and the member lengths of the nested slot are in the facts; main reads a nested leaf and writes another",
       [=](Kit& k)
       {
           StructType* S = mk_s(k);
           StructType* T = k.strukt("T", {{"s", k.st(S)}, {"z", k.i32}});
           Global* g = k.prog.add_global("t", k.st(T), k.at(), nullptr);
           for (const char* v : {"1", "2", "3"})
           {
               GlobalLeaf l;
               l.kind = Global::Init::Value;
               l.init = k.num(k.i32, v);
               l.value = v;
               l.loc = k.at();
               g->leaves.push_back(l);
           }
           Function* m = k.fn("main", k.i32);
           const Expr* ty = k.deref(k.mem(k.mem(ga(k, g), 0), 1)); // t.s.y
           const Stmt* c = k.chk(k.eq_c(ty, k.i32, "2"));
           m->body = k.blk(
               {c, k.lstore(ga(k, g), 1, N(k, "9")), k.chk(k.eq_c(k.leaf(ga(k, g), 1), k.i32, "9")), k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
}

} // namespace
