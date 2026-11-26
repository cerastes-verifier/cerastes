// (included by selftest.cpp) M4.5e1e2 continuation, Part R (record 0064 item 19, question 326): cerastes does not model `restrict`.  The encoder refuses a function that has a `restrict` parameter at the
// C line, so a program never reaches check_program with the flag set; check_program refuses any parameter that carries it, as a backstop.  Each case is one of the shapes the retired rules accepted
// (a cell atom, a range atom, a declaration with no body) and must now be refused, the diagnostic naming the parameter.
namespace {

void define_restrict_backstop_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // int f(int *restrict p) requires(owns(*p)) ensures(owns(*p)) { return 0; }: backed as the retired rule wanted, and refused
    rc("e1e2r_backstop_restrict_backed_by_a_cell", "parameter 1 of 'f' is `restrict`, which cerastes does not model",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // int f(int *restrict p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n)) { return 0; }
    rc("e1e2r_backstop_restrict_backed_by_a_range", "parameter 1 of 'f' is `restrict`, which cerastes does not model",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.land(k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                           k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // int f(int *p, int *restrict q) with no contract and no body (a declaration the unit defines elsewhere): the second parameter, not the first, is named
    rc("e1e2r_backstop_restrict_on_a_second_parameter_of_a_declaration",
       "parameter 2 of 'f' is `restrict`, which cerastes does not model",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"q", PI(k)}})};
           f->params[1U].is_restrict = true;
           k.trusted(f);
           k.done(f);
       });
}

// ---- Part C1 (record 0064 items 7, 8 and 10): ranges over structures ----
// Slice 1: the contract of one function, `owns_range` and `reads_range` over a verified structure, rows per leaf path; a call of such a function and a loop in it are refused by
// check_program until the call rows and the range places are lifted.
void define_struct_range_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // struct P { int x; int y; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"x", k.i32}, {"y", k.i32}});
    };
    auto RO = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    auto pplace = [](Kit& k, const Expr* base, const char* i, int m)
    {
        return k.b.member_addr(k.b.elem_addr(base, k.num(k.i32, i), k.at()), m, k.at());
    };
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == 7) { p[<idx>].x = 7; return p[<idx>].x; }
    const auto write_read = [=](Kit& k, const char* idx) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(k.R(f), N(k, "7"))));
        const Stmt* st {k.store(pplace(k, k.pv(f, 0), idx, 0), N(k, "7"))};
        f->body = k.blkc({st, k.ret(k.deref(pplace(k, k.pv(f, 0), idx, 0)))});
        k.done(f);
        return st->loc;
    };
    // the twins: an index the count does not reach, a write under a read permission, a wrong postcondition
    lc("e1e2c1_range_beyond_the_count_fail",
       "p[2].x with n > 1 only: the bound row fails first (out-of-bounds), and the cell is held by no atom, so the write row fails too (permission), both at the store",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RO(k, f, 0));
           const Stmt* st {k.store(pplace(k, k.pv(f, 0), "2", 0), N(k, "7"))};
           f->body = k.blkc({st});
           k.done(f);
           k.fail("out-of-bounds", st->loc);
           k.fail("permission", st->loc);
       });
    lc("e1e2c1_range_write_under_reads_fail", "reads_range(p, n): p[1].x written: the write row (permission)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RR(k, f, 0));
           const Stmt* st {k.store(pplace(k, k.pv(f, 0), "1", 1), N(k, "7"))};
           f->body = k.blkc({st});
           k.done(f);
           k.fail("permission", st->loc);
       });
    lc("e1e2c1_range_wrong_postcondition_fail",
       "owns_range(p, n): p[1].x = 7 returned, ensures r == 8: the postcondition at the return",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           const Expr* wrong {k.eq(k.R(f), N(k, "8"))};
           k.ens(f, k.land(RO(k, f, 0), wrong));
           const Stmt* ret {k.ret(k.deref(pplace(k, k.pv(f, 0), "1", 0)))};
           f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 0), N(k, "7")), ret});
           k.done(f);
           k.fail("postcondition", ret->loc);
       });
    // ---- more shapes of the contract ----
    // struct Q { int a; struct T { int b; int c; } t; }: a leaf of depth 2 (`p[1].t.c`), the receiver `loc(member(loc(member(loc(block_of(p), k), 1), 0), 1), 0)`
    lc("e1e2c1_range_nested_leaf",
       "owns_range(p, n) over struct Q with a nested struct member: p[1].t.c written and read back through its own leaf path",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
           StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, k.land(RO(k, f, 0), k.eq(k.R(f), N(k, "9"))));
           auto leaf = [&]()
           {
               return k.b.member_addr(k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at()), 1, k.at()), 1,
                                      k.at());
           };
           f->body = k.blkc({k.store(leaf(), N(k, "9")), k.ret(k.deref(leaf()))});
           k.done(f);
       });
    // struct R { int *q; int y; }: a pointer leaf (its range row is the pointer's well-formedness)
    lc("e1e2c1_range_pointer_leaf",
       "owns_range(p, n) over struct R { int *q; int y; }: p[1].y written, the pointer leaf's range is a conjunct of the atom",
       [=](Kit& k)
       {
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RO(k, f, 0));
           f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 1), N(k, "3"))});
           k.done(f);
       });
    // requires(p == NULL || owns_range(p, n)) ensures(the same): the atom under its null guard, nothing is accessed
    lc("e1e2c1_range_null_guard", "p == NULL || owns_range(p, n) in both lists, an empty body",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.lor(k.eq(fx(k, f, 0), k.nul(k.ptr(k.st(P)))), RO(k, f, 0)));
           k.ens(f, k.lor(k.eq(fx(k, f, 0), k.nul(k.ptr(k.st(P)))), RO(k, f, 0)));
           f->body = k.blkc({});
           k.done(f);
       });
    // two reads_range of one structure type (the exception of the checker's neighbour rule): both read
    lc("e1e2c1_range_two_reads_of_one_type",
       "reads_range(p, n) && reads_range(q, m) over struct P, n > 0 && m > 0: q[0].y evaluated and p[0].x returned",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}, {"m", k.i32}})};
           auto rr = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, pi), fx(k, f, ni), amount, k.at());
           };
           k.req(f, k.land(k.land(rr(0, 1, 0), rr(2, 3, 1)),
                           k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
           k.ens(f, k.land(rr(0, 1, 0), rr(2, 3, 1)));
           f->body =
               k.blkc({k.es(k.deref(pplace(k, k.pv(f, 2), "0", 1))), k.ret(k.deref(pplace(k, k.pv(f, 0), "0", 0)))});
           k.done(f);
       });
    // two owned ranges over structures with no leaf field in common (int and long): both written
    lc("e1e2c1_range_two_owns_no_common_field",
       "owns_range(p, n) over struct P { int x; int y; } && owns_range(q, m) over struct L { long l; }: p[0].x and q[0].l written",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           StructType* L {k.strukt("L", {{"l", k.i64}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(L))}, {"m", k.i32}})};
           auto ro = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, pi), fx(k, f, ni), amount, k.at());
           };
           k.req(f, k.land(k.land(ro(0, 1, 0), ro(2, 3, 1)),
                           k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
           k.ens(f, k.land(ro(0, 1, 0), ro(2, 3, 1)));
           f->body = k.blkc(
               {k.store(pplace(k, k.pv(f, 0), "0", 0), N(k, "1")),
                k.store(k.b.member_addr(k.b.elem_addr(k.pv(f, 2), N(k, "0"), k.at()), 0, k.at()), k.num(k.i64, "2"))});
           k.done(f);
       });
    // ---- what check_program keeps refusing until the call rows, the range places and the pairing rows are lifted ----
    // ---- slice 2a: calls and returns (record 0064 item 8; the call rows per leaf path; no pairing yet) ----
    // struct P { int x; int y; }; int g(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == 7) { p[1].x = 7; return p[1].x; }
    auto mk_g = [=](Kit& k, StructType* P, bool owns, int gcount) -> Function*
    {
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto atom = [&](int amount)
        {
            return k.b.range_atom(kind, fx(k, g, 0), fx(k, g, 1), amount, k.at());
        };
        k.req(g, k.land(atom(0), k.gt(fx(k, g, 1), N(k, std::to_string(gcount - 1).c_str()))));
        k.ens(g, k.land(atom(0), k.eq(k.R(g), N(k, "7"))));
        if (owns)
        {
            g->body = k.blkc(
                {k.store(pplace(k, k.pv(g, 0), "1", 0), N(k, "7")), k.ret(k.deref(pplace(k, k.pv(g, 0), "1", 0)))});
        }
        else
        {
            g->body = k.blkc({k.ret(N(k, "7"))});
        }
        k.done(g);
        return g;
    };
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == <want>) { return g(p, n); }
    const auto forward = [=](Kit& k, const char* want, bool owns_in, bool owns_callee) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, owns_callee, 2)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const AtomKind kind {owns_in ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto atom = [&](int amount)
        {
            return k.b.range_atom(kind, fx(k, f, 0), fx(k, f, 1), amount, k.at());
        };
        k.req(f, k.land(atom(0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(atom(0), k.eq(k.R(f), N(k, want))));
        const Expr* c {k.call(g, {k.pv(f, 0), k.pv(f, 1)})};
        const Stmt* ret {k.ret(c)};
        f->body = k.blkc({ret});
        k.done(f);
        return std::string(want) == "7" ? c->loc : ret->loc;
    };
    lc("e1e2c1_call_forward",
       "f(p, n) forwards its owns_range(p, n) over struct P to g, whose contract writes p[1].x: the call rows per leaf path, the callee's postcondition r == 7 reaches f's",
       [=](Kit& k)
       {
           forward(k, "7", true, true);
       });
    lc("e1e2c1_call_forward_wrong_postcondition_fail",
       "the same, f's ensures claims r == 8: the postcondition at f's return",
       [=](Kit& k)
       {
           k.fail("postcondition", forward(k, "8", true, true));
       });
    lc("e1e2c1_call_reads_callee_reads",
       "f holds reads_range(p, n) over struct P and calls g, which reads it: the amounts are fractions of the caller's own",
       [=](Kit& k)
       {
           forward(k, "7", false, false);
       });
    lc("e1e2c1_call_owns_needed_reads_held_fail",
       "f holds reads_range(p, n) and calls g, which needs to own it: the marked rows at write permission fail (permission), the call's row",
       [=](Kit& k)
       {
           const SrcLoc at {forward(k, "7", false, true)};
           // a failed marked row ends its path (S43 G2): Silicon reports the first, the value permission of the member `x`
           k.fail_only("permission", at, 12U, 5U);
       });
    // void f(void) { struct P v[<len>] = {0} or no initializer; int r = g(&v[0], <n>); check(r == 7); }
    const auto local_call = [=](Kit& k, unsigned len, const char* n, bool zero) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, true, 2)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), len), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        auto first = [&]()
        {
            return k.b.elem_addr(k.addr(v), N(k, "0"), k.at());
        };
        const Expr* c {k.call(g, {first(), N(k, n)})};
        Var* r {k.local(f, "r", k.i32)};
        const Stmt* chk {k.chk(k.eq(k.v(r), N(k, "7")))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(r, c), chk});
        k.done(f);
        return c->loc;
    };
    lc("e1e2c1_call_local_array",
       "struct P v[3] = {0}; g(&v[0], 3): the range of the whole local array, then the result r == 7 is known",
       [=](Kit& k)
       {
           local_call(k, 3U, "3", true);
       });
    lc("e1e2c1_call_local_array_too_long_fail",
       "struct P v[3] = {0}; g(&v[0], 4): the shape row of the range (permission at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", local_call(k, 3U, "4", true), 12U, 0U);
       });
    lc("e1e2c1_call_local_array_unset_fail",
       "struct P v[3] with no initializer; g(&v[0], 3): no element has a value, the flag row of each leaf fails (uninitialized, x and y)",
       [=](Kit& k)
       {
           k.fail("uninitialized", local_call(k, 3U, "3", false), 2U);
       });
    lc("e1e2c1_call_owns_to_reads",
       "f owns the range, g only reads it: the callee's marked rows are fractions of the caller's write permission",
       [=](Kit& k)
       {
           // the caller holds the range (owns) and calls a reading callee (a fraction of write)
           StructType* P {mk_p(k)};
           Function* g {mk_g(k, P, false, 2)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, k.land(RO(k, f, 0), k.eq(k.R(f), N(k, "7"))));
           f->body = k.blkc({k.ret(k.call(g, {k.pv(f, 0), k.pv(f, 1)}))});
           k.done(f);
       });
    // int h(struct P *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n) && r == 1) { p[0].x = 1; return 1; }
    auto mk_h = [=](Kit& k, StructType* P) -> Function*
    {
        Function* h {k.fn("h", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(h, k.land(RO(k, h, 0), k.gt(fx(k, h, 1), N(k, "0"))));
        k.ens(h, k.land(RO(k, h, 0), k.eq(k.R(h), N(k, "1"))));
        h->body = k.blkc({k.store(pplace(k, k.pv(h, 0), "0", 0), N(k, "1")), k.ret(N(k, "1"))});
        k.done(h);
        return h;
    };
    // void f(void) { struct P v[3] = {0}; h(&v[0], <n>) == v[1].x; }: the read of v[1].x and the call are unsequenced; the call's range is [0, n)
    const auto seq_read = [=](Kit& k, const char* n) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {mk_h(k, P)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto first = [&]()
        {
            return k.b.elem_addr(k.addr(v), N(k, "0"), k.at());
        };
        const Expr* rd {k.deref(pplace(k, first(), "1", 0))};
        const Expr* c {k.call(h, {first(), N(k, n)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c, rd))});
        k.done(f);
        return rd->loc;
    };
    lc("e1e2c1_seq_call_and_read_disjoint",
       "h(&v[0], 1) == v[1].x: the call's range is [0, 1), the read is element 1: the leaf's row passes",
       [=](Kit& k)
       {
           seq_read(k, "1");
       });
    lc("e1e2c1_seq_call_and_read_overlap_fail",
       "h(&v[0], 2) == v[1].x: the read is a leaf the call's range holds, in an order C leaves unspecified (unsequenced at the read)",
       [=](Kit& k)
       {
           // the read is of the leaf x: the row of the atom of leaf x fails, the row of leaf y holds
           k.fail_only("unsequenced", seq_read(k, "2"), 2U, 0U);
       });
    // void f(void) { struct P v[3] = {0}; h(&v[0], <n>) == h(&v[<m>], 1); }: two calls over one array
    const auto seq_calls = [=](Kit& k, const char* n, const char* at2) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {mk_h(k, P)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto at = [&](const char* i)
        {
            return k.b.elem_addr(k.addr(v), N(k, i), k.at());
        };
        const Expr* c1 {k.call(h, {at("0"), N(k, n)})};
        const Expr* c2 {k.call(h, {at(at2), N(k, "1")})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c1, c2))});
        k.done(f);
        return c2->loc;
    };
    lc("e1e2c1_seq_two_calls_disjoint",
       "h(&v[0], 1) == h(&v[2], 1): two ranges of one element type that share no slot: the slot row passes",
       [=](Kit& k)
       {
           seq_calls(k, "1", "2");
       });
    lc("e1e2c1_seq_two_calls_overlap_fail",
       "h(&v[0], 2) == h(&v[1], 1): the ranges share element 1: unsequenced at the later call",
       [=](Kit& k)
       {
           // per pair of leaf paths of one field: the pair (x, x) fails first and ends the path (Silicon reports one error), (y, y) would fail too, the mixed pairs are true
           k.fail_only("unsequenced", seq_calls(k, "2", "1"), 4U, 0U);
       });
    // trap 3 (a callee's writes through a range over structures are recorded per leaf field): a loop of a caller that keeps v[1].y == 0 across a call
    // void w(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n)) { p[1].y = 5; }
    // void f(void) { struct P v[3] = {0}; int i = 0; while (i < 2) invariant(0 <= i && i <= 2 && v[1].y == 0) decreases(2 - i) { [w(&v[0], 3);] i = i + 1; } }
    const auto loop_call = [=](Kit& k, bool with_call) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* w {k.fn("w", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(w, k.land(RO(k, w, 0), k.gt(fx(k, w, 1), N(k, "1"))));
        k.ens(w, RO(k, w, 0));
        w->body = k.blkc({k.store(pplace(k, k.pv(w, 0), "1", 1), N(k, "5"))});
        k.done(w);
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* i {k.local(f, "i", k.i32)};
        auto first = [&]()
        {
            return k.b.elem_addr(k.addr(v), N(k, "0"), k.at());
        };
        const Expr* y1 {k.deref(pplace(k, k.addr(v), "1", 1))};
        const Stmt* call {k.es(k.call(w, {first(), N(k, "3")}))};
        std::vector<const Stmt*> inner;
        if (with_call)
        {
            inner.push_back(call);
        }
        inner.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Clause keep {k.cl(k.eq(y1, N(k, "0")))};
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.range(k.v(i), k.i32, "0", "2")), keep},
                                {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc(inner))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(i, N(k, "0")), loop});
        k.done(f);
        return keep.loc;
    };
    lc("e1e2c1_loop_around_call_without_call",
       "the same loop without the call: v[1].y == 0 is kept (the contrast of the twin below)",
       [=](Kit& k)
       {
           loop_call(k, false);
       });
    lc("e1e2c1_loop_keeps_value_across_call_fail",
       "the loop calls w, which writes p[1].y through the range: v[1].y == 0 is not kept (the invariant fails at the back edge)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", loop_call(k, true));
       });
    // ---- slice 2b: the pairing rows of record 0064 item 7 (Form P), per pair of leaf paths of one field ----
    // void c2(struct P *p, int n, int *q) requires(<range>(p, n) && <cell>(*q) && n > 0) ensures(<range>(p, n) && <cell>(*q)) {}
    auto mk_c2 = [=](Kit& k, StructType* P, AtomKind range_kind, AtomKind cell_kind) -> Function*
    {
        Function* c {k.fn("c2", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(range_kind, fx(k, c, 0), fx(k, c, 1), amount, k.at());
        };
        auto ca = [&](int amount)
        {
            return k.b.atom(cell_kind, fx(k, c, 2), amount, k.at());
        };
        k.req(c, k.land(k.land(ra(0), ca(1)), k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c, k.land(ra(0), ca(1)));
        c->body = k.blkc({});
        k.done(c);
        return c;
    };
    // void f(void) { struct P v[3] = {0}; int c = 0; c2(&v[0], <n>, <q>); }, q the local's address (qelem < 0) or &v[qelem].<qleaf>
    const auto pair_cell = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind, const char* n, int qelem,
                               int qleaf) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c2 {mk_c2(k, P, range_kind, cell_kind)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* c {k.cell(f, "c", k.i32)};
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* q {qelem < 0 ? k.addr(c) : pplace(k, k.addr(v), std::to_string(qelem).c_str(), qleaf)};
        const Expr* call {k.call(c2, {first, N(k, n), q})};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(c, N(k, "0")), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_cell_outside_range",
       "c2(&v[0], 2, &c) with c a separate local, owns_range(p, n) && owns(*q): the cell is outside the range's leaf cells of x and y: the rows pass",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "2", -1, 0);
       });
    lc("e1e2c1_pair_cell_in_a_later_element",
       "c2(&v[0], 2, &v[2].x): the cell is a leaf of element 2, outside the range [0, 2): the row of leaf x passes",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "2", 2, 0);
       });
    lc("e1e2c1_pair_cell_alias_fail",
       "c2(&v[0], 3, &v[1].x): the cell is leaf x of element 1, which the range holds: two rows (one per leaf), the row of leaf x fails (permission), leaf y's passes",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "3", 1, 0), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_cell_alias_other_leaf_fail",
       "c2(&v[0], 3, &v[2].y): the same through leaf y: the row of leaf y (the second) fails, leaf x's passes",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "3", 2, 1), 2U, 1U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_reads_range_owns_cell_alias_fail",
       "c2 reads the range and owns the cell: c2(&v[0], 3, &v[1].x): an owner among the two sites, the row fails once",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::ReadsRange, AtomKind::Owns, "3", 1, 0), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_reads_range_reads_cell_alias",
       "both sites only read (no row separates them): the cell's sum row (the range's third and the cell's third of write) holds over the caller's full permission",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::ReadsRange, AtomKind::Reads, "3", 1, 0);
       });
    // void f(struct P *p, int n, int *q) requires(owns_range(p, n) && owns(*q) && n > 0) ensures(...) { c2(p, n, q); }: the call's rows are decided from f's own permissions
    lc("e1e2c1_pair_forward_cell",
       "f forwards its owns_range(p, n) and owns(*q) to c2: the cell row of each leaf is proved from f's two full permissions (no alias is possible)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* c2 {mk_c2(k, P, AtomKind::OwnsRange, AtomKind::Owns)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
           auto ra = [&](int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
           };
           auto ca = [&](int amount)
           {
               return k.b.atom(AtomKind::Owns, fx(k, f, 2), amount, k.at());
           };
           k.req(f, k.land(k.land(ra(0), ca(1)), k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, k.land(ra(0), ca(1)));
           f->body = k.blkc({k.es(k.call(c2, {k.pv(f, 0), k.pv(f, 1), k.pv(f, 2)}))});
           k.done(f);
       });
    // void c3(struct P *p, int n, struct P *s) requires(owns_range(p, n) && owns(*s) && n > 0) ensures(the same) {}
    auto mk_c3 = [=](Kit& k, StructType* P, AtomKind range_kind, AtomKind cell_kind) -> Function*
    {
        Function* c {k.fn("c3", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"s", k.ptr(k.st(P))}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(range_kind, fx(k, c, 0), fx(k, c, 1), amount, k.at());
        };
        auto ca = [&](int amount)
        {
            return k.b.atom(cell_kind, fx(k, c, 2), amount, k.at());
        };
        k.req(c, k.land(k.land(ra(0), ca(1)), k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c, k.land(ra(0), ca(1)));
        c->body = k.blkc({});
        k.done(c);
        return c;
    };
    // void f(void) { struct P v[3] = {0}; c3(&v[0], <n>, &v[<s>]); }: a structure atom on an element of the same array
    const auto pair_struct_atom = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind, const char* n,
                                      const char* s) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c3 {mk_c3(k, P, range_kind, cell_kind)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* sp {k.b.elem_addr(k.addr(v), N(k, s), k.at())};
        const Expr* call {k.call(c3, {first, N(k, n), sp})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_struct_atom_outside_range",
       "c3(&v[0], 2, &v[2]): owns(*s) over struct P (leaves x and y) beside owns_range(p, n) over the same type, element 2 outside [0, 2): the cell rows of both leaves pass",
       [=](Kit& k)
       {
           pair_struct_atom(k, AtomKind::OwnsRange, AtomKind::Owns, "2", "2");
       });
    lc("e1e2c1_pair_struct_atom_alias_fail",
       "c3(&v[0], 3, &v[1]): the structure atom's leaves x and y are both cells of element 1: four rows (cell by leaf), the first that fails ends the path (s.x against the range's x)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_struct_atom(k, AtomKind::OwnsRange, AtomKind::Owns, "3", "1"), 4U, 0U,
                       "which may be one object");
       });
    // void h2(struct P *p, int n, struct P *q, int m) requires(<k1>(p, n) && <k2>(q, m) && n > 0 && m > 0) ensures(the same) {}
    auto mk_h2 = [=](Kit& k, StructType* P, AtomKind k1, AtomKind k2) -> Function*
    {
        Function* h {k.fn("h2", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}, {"m", k.i32}})};
        auto ra = [&](AtomKind kind, int pi, int ni, int amount)
        {
            return k.b.range_atom(kind, fx(k, h, pi), fx(k, h, ni), amount, k.at());
        };
        k.req(h, k.land(k.land(ra(k1, 0, 1, 0), ra(k2, 2, 3, 1)),
                        k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
        k.ens(h, k.land(ra(k1, 0, 1, 0), ra(k2, 2, 3, 1)));
        h->body = k.blkc({});
        k.done(h);
        return h;
    };
    // void f(void) { struct P v[3] = {0}; h2(&v[0], <n>, &v[<at2>], <m>); }
    const auto pair_ranges = [=](Kit& k, AtomKind k1, AtomKind k2, const char* n, const char* at2,
                                 const char* m) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {mk_h2(k, P, k1, k2)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto at = [&](const char* i)
        {
            return k.b.elem_addr(k.addr(v), N(k, i), k.at());
        };
        const Expr* call {k.call(h, {at("0"), N(k, n), at(at2), N(k, m)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_two_ranges_disjoint",
       "h2(&v[0], 1, &v[2], 1), two owned ranges over struct P: the slot row passes for (x, x) and (y, y)",
       [=](Kit& k)
       {
           pair_ranges(k, AtomKind::OwnsRange, AtomKind::OwnsRange, "1", "2", "1");
       });
    lc("e1e2c1_pair_two_ranges_overlap_fail",
       "h2(&v[0], 2, &v[1], 1): the ranges share element 1: two rows (x with x, y with y), the first fails (permission) and ends the path",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_ranges(k, AtomKind::OwnsRange, AtomKind::OwnsRange, "2", "1", "1"), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_reads_over_owns_overlap_fail",
       "h2 reads the first range and owns the second: h2(&v[0], 2, &v[1], 1): an owner among the two, the slot row fails",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_ranges(k, AtomKind::ReadsRange, AtomKind::OwnsRange, "2", "1", "1"), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_two_reads_overlap",
       "h2 reads both ranges: h2(&v[0], 2, &v[1], 1) over a local array the caller owns in full: no row separates two readers, the sums of the amounts (each a third of write) hold",
       [=](Kit& k)
       {
           pair_ranges(k, AtomKind::ReadsRange, AtomKind::ReadsRange, "2", "1", "1");
       });
    // void f(struct P *p, int n, struct P *q, int m) requires(owns_range(p, n) && owns_range(q, m) && n > 0 && m > 0) ensures(the same) { h2(p, n, q, m); }
    lc("e1e2c1_pair_forward_two_ranges",
       "f forwards its two owned ranges over struct P to h2: the slot row is proved from f's permissions (no leaf receiver of a first slot is in view: the case the block row could not decide)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* h {mk_h2(k, P, AtomKind::OwnsRange, AtomKind::OwnsRange)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}, {"m", k.i32}})};
           auto ra = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, pi), fx(k, f, ni), amount, k.at());
           };
           k.req(f, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                           k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
           k.ens(f, k.land(ra(0, 1, 0), ra(2, 3, 1)));
           f->body = k.blkc({k.es(k.call(h, {k.pv(f, 0), k.pv(f, 1), k.pv(f, 2), k.pv(f, 3)}))});
           k.done(f);
       });
    // the nested leaf: struct S { int x; struct T { int a; int b; } t; }, a range over S and a range over T at &v[k].t (item 7, "Two ranges": the element structures differ)
    auto mk_nested = [=](Kit& k, StructType*& S, StructType*& T)
    {
        T = k.strukt("T", {{"a", k.i32}, {"b", k.i32}});
        S = k.strukt("S", {{"x", k.i32}, {"t", k.st(T)}});
    };
    // void h4(struct S *p, int n, struct T *q, int m) requires(owns_range(p, n) && owns_range(q, m) && n > 0 && m > 0) ensures(the same) {}
    auto mk_h4 = [=](Kit& k, StructType* S, StructType* T) -> Function*
    {
        Function* h {k.fn("h4", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}, {"q", k.ptr(k.st(T))}, {"m", k.i32}})};
        auto ra = [&](int pi, int ni, int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, h, pi), fx(k, h, ni), amount, k.at());
        };
        k.req(h, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                        k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
        k.ens(h, k.land(ra(0, 1, 0), ra(2, 3, 1)));
        h->body = k.blkc({});
        k.done(h);
        return h;
    };
    // void f(void) { struct S v[3] = {0}; h4(&v[0], <n>, &v[<at2>].t, 1); }
    const auto pair_nested = [=](Kit& k, const char* n, const char* at2) -> SrcLoc
    {
        StructType* S {nullptr};
        StructType* T {nullptr};
        mk_nested(k, S, T);
        Function* h {mk_h4(k, S, T)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* t {k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, at2), k.at()), 1, k.at())};
        const Expr* call {k.call(h, {first, N(k, n), t, N(k, "1")})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_nested_range_outside",
       "h4(&v[0], 2, &v[2].t, 1): a range over struct T at the nested member of element 2, outside the range over struct S [0, 2): the pair of t.a with a and of t.b with b pass",
       [=](Kit& k)
       {
           pair_nested(k, "2", "2");
       });
    lc("e1e2c1_pair_nested_range_alias_fail",
       "h4(&v[0], 3, &v[1].t, 1): the range over T meets the range over S through t.a and t.b: six rows (S's x, t.a, t.b against T's a, b), the first that fails, (t.a, a), ends the path",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_nested(k, "3", "1"), 6U, 2U, "which may be one object");
       });
    // the returns: a function that borrows a range over struct P and a cell of one of its leaf fields (or a range over another element type) returns both: the marked rows' amounts per leaf add what the other atom covers
    lc("e1e2c1_pair_returns_reads_range_and_cell",
       "c2 = reads_range(p, n) over struct P && reads(*q) with an empty body, f of c2's own contract: the return rows add the cell's amount at the range's leaf slots and the range's at the cell",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           mk_c2(k, P, AtomKind::ReadsRange, AtomKind::Reads);
       });
    lc("e1e2c1_pair_returns_two_reads_nested",
       "h4 with reads_range over struct S and reads_range over struct T: the return rows add, per leaf of each range, the other range's amount where its leaves cover the slot",
       [=](Kit& k)
       {
           StructType* S {nullptr};
           StructType* T {nullptr};
           mk_nested(k, S, T);
           Function* h {k.fn("h4r", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}, {"q", k.ptr(k.st(T))}, {"m", k.i32}})};
           auto ra = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::ReadsRange, fx(k, h, pi), fx(k, h, ni), amount, k.at());
           };
           k.req(h, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                           k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
           k.ens(h, k.land(ra(0, 1, 0), ra(2, 3, 1)));
           h->body = k.blkc({});
           k.done(h);
       });
    // void f(void) { struct S v[3] = {0}; h4r(&v[0], 2, &v[1].t, 1); }: both only read, the ranges meet at the nested leaves: the marked rows' per-leaf amounts add the other range's third
    lc("e1e2c1_pair_nested_two_reads_overlap",
       "h4r reads a range over struct S and a range over struct T at &v[1].t, which the S range covers through t.a and t.b: no row separates two readers; the per-leaf marked amounts add the other's, over the caller's full permission",
       [=](Kit& k)
       {
           StructType* S {nullptr};
           StructType* T {nullptr};
           mk_nested(k, S, T);
           Function* h {k.fn("h4r", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}, {"q", k.ptr(k.st(T))}, {"m", k.i32}})};
           auto ra = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::ReadsRange, fx(k, h, pi), fx(k, h, ni), amount, k.at());
           };
           k.req(h, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                           k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
           k.ens(h, k.land(ra(0, 1, 0), ra(2, 3, 1)));
           h->body = k.blkc({});
           k.done(h);
           Function* f {k.fn("f", k.vd)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
           f->mark_address_taken(v);
           v->zero_filled = true;
           const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
           const Expr* t {k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, "1"), k.at()), 1, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(k.call(h, {first, N(k, "2"), t, N(k, "1")}))});
           k.done(f);
       });
    // int hs(struct S *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n) && r == 1) { return 1; }, ht the same over struct T
    // void f(void) { struct S v[3] = {0}; hs(&v[0], <n>) == ht(&v[<at2>].t, 1); }: two calls over ranges of DIFFERENT element types in one expression
    const auto seq_nested = [=](Kit& k, const char* n, const char* at2) -> SrcLoc
    {
        StructType* S {nullptr};
        StructType* T {nullptr};
        mk_nested(k, S, T);
        auto mk_one = [&](const char* name, StructType* E) -> Function*
        {
            Function* h {k.fn(name, k.i32, {{"p", k.ptr(k.st(E))}, {"n", k.i32}})};
            auto ra = [&](int amount)
            {
                return k.b.range_atom(AtomKind::OwnsRange, fx(k, h, 0), fx(k, h, 1), amount, k.at());
            };
            k.req(h, k.land(ra(0), k.gt(fx(k, h, 1), N(k, "0"))));
            k.ens(h, k.land(ra(0), k.eq(k.R(h), N(k, "1"))));
            h->body = k.blkc({k.ret(N(k, "1"))});
            k.done(h);
            return h;
        };
        Function* hs {mk_one("hs", S)};
        Function* ht {mk_one("ht", T)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* t {k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, at2), k.at()), 1, k.at())};
        const Expr* c1 {k.call(hs, {first, N(k, n)})};
        const Expr* c2 {k.call(ht, {t, N(k, "1")})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c1, c2))});
        k.done(f);
        return c2->loc;
    };
    lc("e1e2c1_seq_nested_calls_disjoint",
       "hs(&v[0], 2) == ht(&v[2].t, 1): a range over struct S and a range over struct T at the nested member of element 2: the six leaf pairs pass",
       [=](Kit& k)
       {
           seq_nested(k, "2", "2");
       });
    lc("e1e2c1_seq_nested_calls_overlap_fail",
       "hs(&v[0], 3) == ht(&v[1].t, 1): the T range is the nested member of element 1, which the S range covers: six leaf pairs, the first that fails, (t.a, a), is unsequenced at the later call",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq_nested(k, "3", "1"), 6U, 2U);
       });
    rc("e1e2c1_rej_owns_block_beside_a_struct_range", "an `owns_block` atom beside a range over structures",
       [=](Kit& k)
       {
           // void f(struct P *p, int n, struct P *q) requires(owns_range(p, n) && owns_block(q)): the heap block's cells beside the range are Part D's
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}})};
           k.req(f, k.land(RO(k, f, 0), k.b.atom(AtomKind::OwnsBlock, fx(k, f, 2), -1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_marked_range_contract_read", "is marked `ref_to_uninit` or `must_init` and a contract reads `*p`",
       [=](Kit& k)
       {
           // requires(owns_range(p, n) && n > 0 && p[0].x == 0) on a marked parameter: its elements have no value on entry
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                           k.eq(k.deref(pplace(k, fx(k, f, 0), "0", 0)), N(k, "0"))));
           k.ens(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_marked_range_beside_reads_range", "beside `reads_range(p, n)`",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::MustInit;
           k.req(f, k.land(RO(k, f, 0), RR(k, f, 1)));
           k.ens(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_range_over_a_struct_with_an_array_member", "whose element holds an array member",
       [=](Kit& k)
       {
           StructType* S {k.strukt("S", {{"x", k.i32}, {"a", k.prog.array_of(k.i32, 2U)}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
       });
    // ---- slice 3: element reads `p[e].x` in a contract (record 0064 item 7): framed when the element's slot lies in the range ----
    auto rd = [=](Kit& k, Function* f, const char* idx, int m) -> const Expr*
    {
        return k.b.deref(pplace(k, fx(k, f, 0), idx, m), k.at());
    };
    // int g(struct P *p, int n) requires(<range>(p, n) && n > 1 && p[<ridx>].x == 5) ensures(<range>(p, n) && r == 5) { return p[1].x; }
    const auto elem_req = [=](Kit& k, bool owns, const char* ridx, Function*& out,
                              bool const_body = false) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto atom = [&](int amount)
        {
            return k.b.range_atom(kind, fx(k, g, 0), fx(k, g, 1), amount, k.at());
        };
        const Expr* read {rd(k, g, ridx, 0)};
        k.req(g, k.land(k.land(atom(0), k.gt(fx(k, g, 1), N(k, "1"))), k.eq(read, N(k, "5"))));
        k.ens(g, k.land(atom(0), k.eq(k.R(g), N(k, "5"))));
        g->body = k.blkc({k.ret(const_body ? N(k, "5") : k.deref(pplace(k, k.pv(g, 0), "1", 0)))});
        k.done(g);
        out = g;
        return read;
    };
    lc("e1e2c1_elem_requires_reads",
       "reads_range(p, n) && n > 1 && p[1].x == 5 in the requires: the read is framed by the range (element 1 is inside [0, n)), the body returns 5",
       [=](Kit& k)
       {
           Function* g {nullptr};
           elem_req(k, false, "1", g);
       });
    lc("e1e2c1_elem_requires_owns", "the same over owns_range(p, n)",
       [=](Kit& k)
       {
           Function* g {nullptr};
           elem_req(k, true, "1", g);
       });
    lc("e1e2c1_elem_requires_outside_fail",
       "p[2].x == 5 in the requires with only n > 1: the read of element 2 is framed by no atom (permission, f__wf's framing row)",
       [=](Kit& k)
       {
           Function* g {nullptr};
           k.fail("permission", elem_req(k, false, "2", g, true)->loc);
       });
    // void f(void) { struct P v[3] = {0}; v[1].x = <val>; g(&v[0], 3); }
    const auto elem_caller = [=](Kit& k, const char* val) -> SrcLoc
    {
        Function* g {nullptr};
        elem_req(k, true, "1", g);
        Function* f {k.fn("f", k.vd)};
        StructType* P {const_cast<StructType*>(g->params[0U].type.pointee->struct_type)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Stmt* st {k.store(pplace(k, k.addr(v), "1", 0), N(k, val))};
        const Expr* c {k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "3")})};
        f->body = k.blkc({k.decl(v, nullptr), st, k.es(c)});
        k.done(f);
        return c->loc;
    };
    lc("e1e2c1_elem_call_holds",
       "g needs p[1].x == 5: the caller wrote v[1].x = 5, the callee's requires holds at the call",
       [=](Kit& k)
       {
           elem_caller(k, "5");
       });
    lc("e1e2c1_elem_call_fails",
       "the caller wrote v[1].x = 4: the callee's requires p[1].x == 5 fails at the call (precondition)",
       [=](Kit& k)
       {
           k.fail("precondition", elem_caller(k, "4"));
       });
    // void f(struct P *p, int n, int i) requires(owns_range(p, n) && 0 <= i && i <= <bound> n... && p[i].y > 0)
    const auto elem_index = [=](Kit& k, bool strict) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"i", k.i32}})};
        const Expr* read {
            k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), fx(k, f, 2), k.at()), 1, k.at()), k.at())};
        const Expr* below {strict ? k.lt(fx(k, f, 2), fx(k, f, 1)) : k.le(fx(k, f, 2), fx(k, f, 1))};
        k.req(f, k.land(k.land(RO(k, f, 0), k.land(k.ge(fx(k, f, 2), N(k, "0")), below)), k.gt(read, N(k, "0"))));
        k.ens(f, RO(k, f, 0));
        f->body = k.blkc({});
        k.done(f);
        return read;
    };
    lc("e1e2c1_elem_index_inside", "0 <= i && i < n && p[i].y > 0: the read at a symbolic index is framed",
       [=](Kit& k)
       {
           elem_index(k, true);
       });
    lc("e1e2c1_elem_index_one_past_fail", "0 <= i && i <= n && p[i].y > 0: i == n is outside the range (permission)",
       [=](Kit& k)
       {
           k.fail("permission", elem_index(k, false)->loc);
       });
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && p[1].x == <want>) { p[1].x = 7; return 0; }
    const auto elem_ens = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(rd(k, f, "1", 0), N(k, want))));
        const Stmt* ret {k.ret(N(k, "0"))};
        f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 0), N(k, "7")), ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c1_elem_ensures_after_write",
       "ensures p[1].x == 7 after p[1].x = 7: the post-state read is framed by the ensures' range",
       [=](Kit& k)
       {
           elem_ens(k, "7");
       });
    lc("e1e2c1_elem_ensures_wrong_fail", "ensures p[1].x == 8 after p[1].x = 7: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", elem_ens(k, "8"));
       });
    // void f(struct P *p, int n) requires(owns_range(p, n) && n > 1 && p[1].x == 4) ensures(owns_range(p, n) && p[1].x == old(p[1].x) + <step>) { p[1].x = 5; }
    const auto elem_old = [=](Kit& k, const char* step) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(rd(k, f, "1", 0), N(k, "4"))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(rd(k, f, "1", 0), k.add(k.old(rd(k, f, "1", 0)), N(k, step)))));
        const Stmt* ret {k.ret(N(k, "0"))};
        f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 0), N(k, "5")), ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c1_elem_old", "p[1].x == old(p[1].x) + 1 after p[1].x = 5 with p[1].x == 4 on entry",
       [=](Kit& k)
       {
           elem_old(k, "1");
       });
    lc("e1e2c1_elem_old_wrong_fail", "the same with + 2: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", elem_old(k, "2"));
       });
    // a nested leaf and a pointer leaf of the element
    lc("e1e2c1_elem_nested_leaf",
       "struct Q { int a; struct T { int b; int c; } t; }: requires(reads_range(p, n) && n > 1 && p[1].t.c == 4) ensures(reads_range(p, n) && p[1].t.c == 4)",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
           StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           auto leaf = [&]()
           {
               return k.b.deref(
                   k.b.member_addr(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "1"), k.at()), 1, k.at()), 1,
                                   k.at()),
                   k.at());
           };
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(leaf(), N(k, "4"))));
           k.ens(f, k.land(RR(k, f, 0), k.eq(leaf(), N(k, "4"))));
           f->body = k.blkc({});
           k.done(f);
       });
    lc("e1e2c1_elem_pointer_leaf",
       "struct R { int *q; int y; }: requires(reads_range(p, n) && n > 0 && p[0].q != NULL) ensures(reads_range(p, n)): a pointer leaf's value is read",
       [=](Kit& k)
       {
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()), k.at())};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))), k.ne(q, k.nul(k.ptr(k.i32)))));
           k.ens(f, RR(k, f, 0));
           f->body = k.blkc({});
           k.done(f);
       });
    // framed by a structure atom whose place is the element's slot: void f(struct P *p, struct P *q) requires(owns(*q) && p == q && p[0].x == 1)
    const auto elem_by_atom = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"q", k.ptr(k.st(P))}})};
        const Expr* read {rd(k, f, "0", 0)};
        const Expr* own {k.b.atom(AtomKind::Owns, fx(k, f, 1), 0, k.at())};
        const Expr* eqpq {k.eq(fx(k, f, 0), fx(k, f, 1))};
        k.req(f, alias ? k.land(k.land(own, eqpq), k.eq(read, N(k, "1"))) : k.land(own, k.eq(read, N(k, "1"))));
        k.ens(f, k.b.atom(AtomKind::Owns, fx(k, f, 1), 0, k.at()));
        f->body = k.blkc({});
        k.done(f);
        return read;
    };
    lc("e1e2c1_elem_framed_by_a_struct_atom",
       "owns(*q) && p == q && p[0].x == 1: the read of p's element 0 is the leaf of q's structure",
       [=](Kit& k)
       {
           elem_by_atom(k, true);
       });
    lc("e1e2c1_elem_struct_atom_elsewhere_fail",
       "owns(*q) && p[0].x == 1 with no p == q: the read is framed by no atom (permission)",
       [=](Kit& k)
       {
           k.fail("permission", elem_by_atom(k, false)->loc);
       });
    // decreases(p[<d>].y) with requires(reads_range(p, n) && n > 1 && p[1].y >= 0)
    const auto elem_dec = [=](Kit& k, const char* d) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.ge(rd(k, f, "1", 1), N(k, "0"))));
        k.ens(f, RR(k, f, 0));
        const Expr* m {rd(k, f, d, 1)};
        k.dec(f, m);
        f->body = k.blkc({});
        k.done(f);
        return m;
    };
    lc("e1e2c1_elem_decreases",
       "decreases(p[1].y) with reads_range(p, n) && n > 1 && p[1].y >= 0: the measure's read is framed",
       [=](Kit& k)
       {
           elem_dec(k, "1");
       });
    lc("e1e2c1_elem_decreases_outside_fail",
       "decreases(p[5].y): the measure's read is outside the range (permission, the measure's wording)",
       [=](Kit& k)
       {
           k.fail("permission", elem_dec(k, "5")->loc);
       });
    // what the lift keeps refused: a pointer loaded from an element, then read or indexed; an element's address as a value
    rc("e1e2c1_rej_elem_pointer_leaf_pointee",
       "a dereference in a contract of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && n > 0 && *p[0].q == 1)
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()), k.at())};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))), k.eq(k.deref(q), N(k, "1"))));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_elem_pointer_leaf_index", "an element read in a contract of anything but a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && n > 0 && p[0].q[0] == 1)
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()), k.at())};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                           k.eq(k.deref(k.b.elem_addr(q, N(k, "0"), k.at())), N(k, "1"))));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_elem_address_value", "the address of a member in a contract outside the place of a read",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && n > 1 && &p[1].x == &p[1].x)
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           const Expr* a {pplace(k, fx(k, f, 0), "1", 0)};
           const Expr* b2 {pplace(k, fx(k, f, 0), "1", 0)};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(a, b2)));
           f->body = k.blk({});
           k.done(f);
       });
    // ---- slice 4: loops over a parameter's range of structures (record 0064 item 10): a range place per leaf path ----
    // the parameter's local copy in a body, `p[e].<m>` there and the store, increment and bound helpers
    auto bat = [=](Kit& k, Function* f, int pi, const Expr* idx, int m) -> const Expr*
    {
        return k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(pi)), idx, k.at()), m, k.at());
    };
    auto brd = [=](Kit& k, Function* f, int pi, const Expr* idx, int m) -> const Expr*
    {
        return k.deref(bat(k, f, pi, idx, m));
    };
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
    };
    auto bounds = [=](Kit& k, Function* f, Var* i, int n)
    {
        return k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), k.pv(f, n)));
    };
    auto vac = [](Kit& k, const Stmt* c)
    {
        k.fail("check", c->loc);
    };
    // void fill(struct P *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while ([i < n|i <= n]) inv 0 <= i <= n decreases n - i { p[i].x = i; [check(false);] i++; } [check(false);] }
    // `mode`: 0 the fill, 1 i <= n, 2 check(false) after the loop, 3 check(false) at the end of the body
    const auto lfill = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {k.store(bat(k, f, 0, k.v(i), 0), k.v(i))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c3 != nullptr)
        {
            bs.push_back(c3);
        }
        bs.push_back(inc(k, i));
        const Expr* cond {mode == 1 ? k.le(k.v(i), k.pv(f, 1)) : k.lt(k.v(i), k.pv(f, 1))};
        const Stmt* loop {k.whl(cond, {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 2 ? c2 : mode == 3 ? c3 : w;
    };
    lc("e1e2c1_loop_fill",
       "while (i < n) p[i].x = i over owns_range(p, n) of struct P: one range place per leaf (x and y) carries the permissions, flags and values through the loop",
       [=](Kit& k)
       {
           lfill(k, 0);
       });
    lc("e1e2c1_loop_fill_high",
       "the loop runs to i <= n: p[n].x fails its bound row and its write-permission row at the store",
       [=](Kit& k)
       {
           const Stmt* w {lfill(k, 1)};
           k.fail("out-of-bounds", w->loc);
           k.fail("permission", w->loc);
       });
    lc("e1e2c1_loop_fill_vacuity", "the fill with check(false) after the loop: it fails (the loop's path is not empty)",
       [=](Kit& k)
       {
           vac(k, lfill(k, 2));
       });
    lc("e1e2c1_loop_fill_body_vacuity", "the fill with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lfill(k, 3));
       });
    // int vals(struct <S> *p, int n) requires(<range>(p, n) && n > 0 && p[0].<rm> == 7) ensures(<range>(p, n) && r == 0)
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { [p[i].<wm> = 0;] i++; } check(p[0].<rm> == <want>); return 0; }
    // S is struct P { int x; int y; } or struct L { long l; int y; }; `wm` < 0: the loop writes nothing
    const auto lvals = [=](Kit& k, bool longs, bool reads, int wm, int rm, const char* want) -> const Stmt*
    {
        StructType* S {longs ? k.strukt("L", {{"l", k.i64}, {"y", k.i32}}) : mk_p(k)};
        Function* f {k.fn("vals", k.i32, {{"p", k.ptr(k.st(S))}, {"n", k.i32}})};
        auto atom = [&](int amount)
        {
            return reads ? RR(k, f, amount) : RO(k, f, amount);
        };
        const Expr* seven {
            k.eq(k.deref(pplace(k, fx(k, f, 0), "0", rm)), longs && rm == 0 ? k.num(k.i64, "7") : N(k, "7"))};
        k.req(f, k.land(k.land(atom(0), k.gt(fx(k, f, 1), N(k, "0"))), seven));
        k.ens(f, k.land(atom(0), k.eq(k.R(f), N(k, "0"))));
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> bs;
        if (wm >= 0)
        {
            bs.push_back(k.store(bat(k, f, 0, k.v(i), wm), N(k, "0")));
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c {k.chk(k.eq(brd(k, f, 0, N(k, "0"), rm), longs && rm == 0 ? k.num(k.i64, want) : N(k, want)))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, c, k.ret(N(k, "0"))});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_values_kept_reads",
       "a read-only loop over reads_range(p, n) of struct P keeps the values: p[0].x == 7 still holds after it",
       [=](Kit& k)
       {
           lvals(k, false, true, -1, 0, "7");
       });
    lc("e1e2c1_loop_values_kept_owns",
       "a loop over owns_range(p, n) that writes nothing keeps the values (no write reaches the field)",
       [=](Kit& k)
       {
           lvals(k, false, false, -1, 0, "7");
       });
    lc("e1e2c1_loop_values_wrong_fail", "the same checking p[0].x == 8: fails at the check",
       [=](Kit& k)
       {
           vac(k, lvals(k, false, true, -1, 0, "8"));
       });
    lc("e1e2c1_loop_values_lost_same_field_fail",
       "the loop writes p[i].y (an int): it reaches the int field, so p[0].x == 7 is lost though the leaf differs (the listed false alarm of record 0064 item 10: the check fails)",
       [=](Kit& k)
       {
           vac(k, lvals(k, false, false, 1, 0, "7"));
       });
    lc("e1e2c1_loop_values_kept_other_field",
       "struct L { long l; int y; }: the loop writes p[i].y (an int), the long field l is not reached: p[0].l == 7 is kept",
       [=](Kit& k)
       {
           lvals(k, true, false, 1, 0, "7");
       });
    // void countdown(struct P *p, int n) requires(owns_range(p, n) && n > 0 && 0 <= p[0].x && p[0].x <= 100) ensures(owns_range(p, n) && p[0].x == 0)
    //   { while (p[0].x > 0) inv 0 <= p[0].x && p[0].x <= 100 decreases p[0].x { p[0].x -= 1; [check(false);] } [check(false);] }
    const auto lcountdown = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("countdown", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                        k.land(k.ge(rd(k, f, "0", 0), N(k, "0")), k.le(rd(k, f, "0", 0), N(k, "100")))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(rd(k, f, "0", 0), N(k, "0"))));
        const Stmt* w {k.cstore(bat(k, f, 0, N(k, "0"), 0), BinaryOp::Sub, N(k, "1"))};
        const Stmt* c1 {mode == 1 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c1 != nullptr)
        {
            bs.push_back(c1);
        }
        const Stmt* loop {k.whl(
            k.gt(brd(k, f, 0, N(k, "0"), 0), N(k, "0")),
            {k.cl(k.land(k.ge(brd(k, f, 0, N(k, "0"), 0), N(k, "0")), k.le(brd(k, f, 0, N(k, "0"), 0), N(k, "100"))))},
            {k.cl(brd(k, f, 0, N(k, "0"), 0))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 1 ? c1 : c2;
    };
    lc("e1e2c1_loop_element_clauses",
       "p[0].x in a loop's invariant and in its measure over owns_range(p, n) of struct P: the reads are framed by the range place's leaf by the slot of the pointer's value",
       [=](Kit& k)
       {
           lcountdown(k, 0);
       });
    lc("e1e2c1_loop_element_clauses_body_vacuity", "the countdown with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lcountdown(k, 1));
       });
    lc("e1e2c1_loop_element_clauses_vacuity", "the countdown with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac(k, lcountdown(k, 2));
       });
    // int first_negative(struct P *p, int n) requires(reads_range(p, n)) ensures(reads_range(p, n) && (r == -1 || (0 <= r && r < n && p[r].x < 0)))
    //   { int i = 0; while (i < n) inv 0 <= i <= n && (i == 0 || p[i - 1].x >= 0) decreases n - i { if (p[i].x < 0) { return i; } i++; } return -1; }
    // `unframed`: the invariant is `i < n || p[i].x >= 0`, which reads p[i].x where i is n
    const auto lfirst = [=](Kit& k, bool unframed) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("first_negative", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0));
        auto cread = [&](const Expr* idx)
        {
            return k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), idx, k.at()), 0, k.at()));
        };
        k.ens(f, k.land(RR(k, f, 0), k.lor(k.eq(k.R(f), N(k, "-1")),
                                           k.land(k.land(k.ge(k.R(f), N(k, "0")), k.lt(k.R(f), fx(k, f, 1))),
                                                  k.lt(cread(k.R(f)), N(k, "0"))))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* read {k.b.text(brd(k, f, 0, unframed ? k.v(i) : k.sub(k.v(i), N(k, "1")), 0),
                                   unframed ? "p[i].x" : "p[i - 1].x")};
        const Expr* clause {
            k.lor(unframed ? k.lt(k.v(i), k.pv(f, 1)) : k.eq(k.v(i), N(k, "0")), k.ge(read, N(k, "0")))};
        const Stmt* body {
            k.blkc({k.iff(k.lt(brd(k, f, 0, k.v(i), 0), N(k, "0")), k.blkc({k.ret(k.v(i))}), nullptr), inc(k, i)})};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), k.cl(clause)},
                                {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(N(k, "-1"))});
        k.done(f);
        return read;
    };
    lc("e1e2c1_loop_element_guarded",
       "p[i - 1].x under i == 0 || in a loop invariant over reads_range(p, n): framed by the range for 0 < i <= n",
       [=](Kit& k)
       {
           lfirst(k, false);
       });
    lc("e1e2c1_loop_element_unframed_fail",
       "i < n || p[i].x >= 0 as an invariant: for i == n (and for n == 0 at the entry) the range place ends at n, the read is unframed (permission at the read in the clause)",
       [=](Kit& k)
       {
           k.fail("permission", lfirst(k, true)->loc, 2U, "", "this loop invariant reads `p[i].x`");
       });
    // int pin(struct P *p, struct P *q, int n) requires(reads_range(p, n) [&& reads_range(q, n) && q[0].x == 4] && n > 0) ensures(the same)
    //   { int i = 0; p = q; while (i < n) inv 0 <= i <= n && <p|q>[0].x == 4 decreases n - i { i++; } return 0; }
    // `mode`: 0 the invariant reads q[0].x, 1 it reads p[0].x through the assigned p (= q), 2 the same with only live(q) on q
    const auto lpin = [=](Kit& k, int mode) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("pin", k.i32, {{"p", k.ptr(k.st(P))}, {"q", k.ptr(k.st(P))}, {"n", k.i32}})};
        auto rr = [&](int pi, int amount)
        {
            return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, pi), fx(k, f, 2), amount, k.at());
        };
        const Expr* q4 {
            k.eq(k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 1), N(k, "0"), k.at()), 0, k.at())), N(k, "4"))};
        if (mode == 2)
        {
            k.req(f, k.land(k.land(rr(0, 0), k.b.atom(AtomKind::Live, fx(k, f, 1), 1, k.at())),
                            k.gt(fx(k, f, 2), N(k, "0"))));
            k.ens(f, k.land(rr(0, 0), k.b.atom(AtomKind::Live, fx(k, f, 1), 1, k.at())));
        }
        else
        {
            k.req(f, k.land(k.land(k.land(rr(0, 0), rr(1, 1)), k.gt(fx(k, f, 2), N(k, "0"))), q4));
            k.ens(f, k.land(rr(0, 0), rr(1, 1)));
        }
        Var* i {k.local(f, "i", k.i32)};
        const Expr* read {k.b.text(brd(k, f, mode == 0 ? 1 : 0, N(k, "0"), 0), mode == 0 ? "q[0].x" : "p[0].x")};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 2)), {k.cl(bounds(k, f, i, 2)), k.cl(k.eq(read, N(k, "4")))},
                                {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, k.blkc({inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), k.asg(f->param_var(0), k.pn(f->param_var(1))), loop, k.ret(N(k, "0"))});
        k.done(f);
        return read;
    };
    lc("e1e2c1_loop_assigned_param_other_range",
       "the function assigns p (p = q) and the loop's invariant reads q[0].x: q's range place carries it (p has none), the value is kept",
       [=](Kit& k)
       {
           lpin(k, 0);
       });
    lc("e1e2c1_loop_assigned_param_reads_the_value",
       "the same reading p[0].x through the assigned p: the read is by the slot of p's CURRENT value (q's), which q's range place holds, so it is framed",
       [=](Kit& k)
       {
           lpin(k, 1);
       });
    lc("e1e2c1_loop_assigned_param_unframed_fail",
       "p = q with only live(q) held for q (no range on q), the invariant reads p[0].x: p has no range place and q's slot is held by none, so the read is unframed (permission at the read in the clause, at the loop's entry: the invariant then assumes the false framing), never framed by p's entry range",
       [=](Kit& k)
       {
           k.fail_only("permission", lpin(k, 2)->loc, 2U, 0U, "this loop invariant reads `p[0].x`");
       });
    // int scan(struct P *p, int n, int *q, int m) requires(reads_range(p, n) && reads_range(q, m) && n > 0 && m > 0) ensures(the same)
    //   { int i = 0; int s = 0; while (i < n) inv 0 <= i <= n decreases n - i { [p[0].x = p[0].x;] if (p[i].x == q[0]) { s++; } i++; } return 0; }
    // a range over structures beside a range of ints: each leaf place of p carries the amount of q where q covers its cells (the mixed row of range_row_amount)
    const auto ltwo = [=](Kit& k, bool write) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("scan", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}, {"m", k.i32}})};
        const Expr* rp {k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at())};
        const Expr* rq {k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 2), fx(k, f, 3), 1, k.at())};
        k.req(f, k.land(k.land(rp, rq), k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
        k.ens(f, k.land(k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                        k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 2), fx(k, f, 3), 1, k.at())));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {write ? k.store(bat(k, f, 0, N(k, "0"), 0), brd(k, f, 0, N(k, "0"), 0)) : nullptr};
        std::vector<const Stmt*> bs;
        if (w != nullptr)
        {
            bs.push_back(w);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return w;
    };
    lc("e1e2c1_loop_two_reads",
       "reads_range over struct P beside reads_range over int: each leaf place of p carries the amount of q where q covers its cells (the mixed row of range_row_amount), the loop reads nothing",
       [=](Kit& k)
       {
           ltwo(k, false);
       });
    lc("e1e2c1_loop_two_reads_write_fail",
       "the loop writes p[0].x = p[0].x under reads_range(p, n) (the same value, so the values-kept conjunct holds): the write row fails (permission)",
       [=](Kit& k)
       {
           k.fail("permission", ltwo(k, true)->loc);
       });
    // struct Q { int a; struct T { int b; int c; } t; }: void fillq(struct Q *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i].t.c = i; [check(false);] i++; } }
    const auto lnested = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
        StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
        Function* f {k.fn("fillq", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* leaf {k.b.member_addr(
            k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), k.v(i), k.at()), 1, k.at()), 1, k.at())};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.store(leaf, k.v(i))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_nested_leaf",
       "struct Q { int a; struct T { int b; int c; } t; }: the loop writes p[i].t.c, one range place per leaf (a, t.b, t.c) of depth 1 and 2",
       [=](Kit& k)
       {
           lnested(k, false);
       });
    lc("e1e2c1_loop_nested_leaf_body_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lnested(k, true));
       });
    // void beside(struct P *p, int n, int *q) requires(owns_range(p, n) && owns(*q) && n > 0) ensures(owns_range(p, n) && owns(*q))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { *q = i; p[i].x = i; [check(false);] i++; } }
    const auto lbeside = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("beside", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
        auto cell = [&]()
        {
            return k.b.atom(AtomKind::Owns, fx(k, f, 2), 1, k.at());
        };
        k.req(f, k.land(k.land(RO(k, f, 0), cell()), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(RO(k, f, 0), cell()));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.store(k.pn(f->param_var(2)), k.v(i)), k.store(bat(k, f, 0, k.v(i), 0), k.v(i))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_beside_cell",
       "owns_range(p, n) over struct P beside owns(*q) of an int: the loop writes *q and p[i].x; the cell place is held apart from the range's leaf places (the guards and case splits per leaf)",
       [=](Kit& k)
       {
           lbeside(k, false);
       });
    lc("e1e2c1_loop_beside_cell_body_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lbeside(k, true));
       });
    // void nest(struct P *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { int j = 0; while (j < n) inv 0 <= j <= n decreases n - j { p[j].y = i; [check(false);] j++; } i++; } }
    const auto lnest = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("nest", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib {k.store(bat(k, f, 0, k.v(j), 1), k.v(i))};
        if (c != nullptr)
        {
            ib.push_back(c);
        }
        ib.push_back(inc(k, j));
        const Stmt* inner {
            k.whl(k.lt(k.v(j), k.pv(f, 1)), {k.cl(bounds(k, f, j, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(j)))}, k.blkc(ib))};
        const Stmt* outer {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))},
                                 {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({k.decl(j, N(k, "0")), inner, inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), outer});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_nested_loops",
       "two nested loops over one owned range of structures: each carries every leaf place, its own label and rows",
       [=](Kit& k)
       {
           lnest(k, false);
       });
    lc("e1e2c1_loop_nested_loops_vacuity", "the same with check(false) at the end of the inner body: it fails",
       [=](Kit& k)
       {
           vac(k, lnest(k, true));
       });
    // int g(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == 7) { p[1].x = 7; return p[1].x; }  (mk_g)
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n)) { int i = 0; while (i < 2) inv 0 <= i <= 2 decreases 2 - i { [p[1].y = 5;] g(p, n); [check(false);] i++; } return 0; }
    const auto lcall = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, true, 2)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.es(k.call(g, {k.pn(f->param_var(0)), k.pn(f->param_var(1))}))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "2")),
                                {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2"))))},
                                {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_calls_a_range_function",
       "the loop calls g(p, n), which takes the whole range and gives it back: the range places' marked rows hold at the back edge",
       [=](Kit& k)
       {
           lcall(k, false);
       });
    lc("e1e2c1_loop_calls_a_range_function_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           vac(k, lcall(k, true));
       });
    // what stays refused or unframed with loaded pointers: `owns(*p[0].q)`, `owns_block(p[0].q)` in a contract; `&p[1].x` as a value and `p[0].q[0]` in a loop invariant
    const auto mk_r = [=](Kit& k) -> StructType*
    {
        return k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}});
    };
    rc("e1e2c1_rej_atom_through_an_element_pointer", "a permission atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && owns(*p[0].q))
           StructType* R {mk_r(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()))};
           k.req(f, k.land(RR(k, f, 0), k.b.atom(AtomKind::Owns, q, 1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_owns_block_through_an_element_pointer",
       "an `owns_block` atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && owns_block(p[0].q))
           StructType* R {mk_r(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()))};
           k.req(f, k.land(RR(k, f, 0), k.b.atom(AtomKind::OwnsBlock, q, -1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    // void f(struct R *p, int n) requires(reads_range(p, n) && n > 0) ensures(reads_range(p, n)) { int i = 0; while (i < n) inv 0 <= i <= n && [p[0].q[0] >= 0 | &p[1].x == &p[1].x] decreases n - i { i++; } }
    const auto lptr = [=](Kit& k, bool address) -> const Expr*
    {
        StructType* R {mk_r(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, RR(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* leaf {k.deref(k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "0"), k.at()), 0, k.at()))};
        const Expr* clause {nullptr};
        const Expr* read {nullptr};
        if (address)
        {
            clause = k.eq(k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "1"), k.at()), 1, k.at()),
                          k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "1"), k.at()), 1, k.at()));
        }
        else
        {
            read = k.b.text(k.deref(k.b.elem_addr(leaf, N(k, "0"), k.at())), "p[0].q[0]");
            clause = k.ge(read, N(k, "0"));
        }
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), k.cl(clause)},
                                {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return address ? clause : read;
    };
    rc("e1e2c1_rej_loop_element_pointer_pointee", "a read through a pointer loaded from a member of an element",
       [=](Kit& k)
       {
           lptr(k, false);
       });
    rc("e1e2c1_rej_loop_element_address_value",
       "the element of a pointer to a structure in a loop invariant or measure",
       [=](Kit& k)
       {
           lptr(k, true);
       });
    // void f(void) { struct S v[3] = {0}; h2(&v[0], <n>, &v[<at2>], <m>); } with h2 over struct S { int x; struct T { int a; int b; } t; }: three leaves of paths of length 1 and 2, so the pairs
    // (x, t.a) take the general row of record 0064 item 7 and (t.a, t.b) the injectivity of `member`
    const auto pair_s = [=](Kit& k, const char* n, const char* at2, const char* m) -> SrcLoc
    {
        StructType* S {nullptr};
        StructType* T {nullptr};
        mk_nested(k, S, T);
        Function* h {mk_h2(k, S, AtomKind::OwnsRange, AtomKind::OwnsRange)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto at = [&](const char* idx)
        {
            return k.b.elem_addr(k.addr(v), N(k, idx), k.at());
        };
        const Expr* call {k.call(h, {at("0"), N(k, n), at(at2), N(k, m)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_nested_s_ranges_disjoint",
       "h2(&v[0], 1, &v[2], 1) over struct S with a nested member: the rows of the pairs of leaves (seven: the two pairs of distinct leaves of one length are `true`; different lengths included) pass",
       [=](Kit& k)
       {
           pair_s(k, "1", "2", "1");
       });
    lc("e1e2c1_pair_nested_s_ranges_overlap_fail",
       "h2(&v[0], 2, &v[1], 1): element 1 is in both ranges; the first pair that meets (x, x) fails (permission) among seven rows",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_s(k, "2", "1", "1"), 7U, 0U, "which may be one object");
       });
    // void c2v(struct P *p, int n, int **pq) requires(owns_range(p, n) && owns(*pq) && owns(**pq) && n > 0) ensures(the same && *pq == old(*pq)) {}
    // void f(void) { struct P v[3] = {0}; int c = 0; int *pp = &<c|v[1].x>; c2v(&v[0], 3, &pp); }: a cell reached through a load beside a range over structures
    const auto pair_via = [=](Kit& k, bool alias) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c {k.fn("c2v", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"pq", k.ptr(k.ptr(k.i32))}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, c, 0), fx(k, c, 1), amount, k.at());
        };
        auto cell = [&](int amount)
        {
            return k.b.atom(AtomKind::Owns, fx(k, c, 2), amount, k.at());
        };
        auto via = [&](int amount)
        {
            return k.b.atom(AtomKind::Owns, k.deref(fx(k, c, 2)), amount, k.at());
        };
        k.req(c, k.land(k.land(k.land(ra(0), cell(1)), via(2)), k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c,
              k.land(k.land(k.land(ra(0), cell(1)), via(2)), k.eq(k.deref(fx(k, c, 2)), k.old(k.deref(fx(k, c, 2))))));
        c->body = k.blkc({});
        k.done(c);
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* cc {k.cell(f, "c", k.i32)};
        Var* pp {k.cell(f, "pp", k.ptr(k.i32))};
        const Expr* target {alias ? pplace(k, k.addr(v), "1", 0) : k.addr(cc)};
        const Expr* call {k.call(c, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "3"), k.addr(pp)})};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(cc, N(k, "0")), k.decl(pp, target), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_via_cell_outside_range",
       "c2v(&v[0], 3, &pp) with pp = &c: owns(**pq) names the separate cell c, outside the range's leaf cells: the rows pass",
       [=](Kit& k)
       {
           pair_via(k, false);
       });
    lc("e1e2c1_pair_via_cell_alias_fail",
       "pp = &v[1].x: the cell reached through the load is a leaf of element 1, which the range holds: the row of leaf x fails (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_via(k, true), 2U, 0U, "which may be one object");
       });
    lc("e1e2c1_range_write_read",
       "owns_range(p, n) over struct P, n > 1: p[1].x written and read back, the ensures keeps the range",
       [=](Kit& k)
       {
           write_read(k, "1");
       });
    // int f(struct P *p, int n) requires(reads_range(p, n) && n > 1) ensures(reads_range(p, n)) { return p[1].y; }
    lc("e1e2c1_range_read_only",
       "reads_range(p, n) over struct P, n > 1: p[1].y read under the ghost amount, the ensures returns the range",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RR(k, f, 0));
           f->body = k.blkc({k.ret(k.deref(pplace(k, k.pv(f, 0), "1", 1)))});
           k.done(f);
       });
    // ---- slice 5 (record 0064 items 7, 8 and 10; record 0060 items 12 and 13): `owning(n)` and the markers over a range of structures ----
    // `owning(n)` on `struct P *p` is `p == NULL || owns_range(p, n)` in both lists (0060 item 13): the atom under its guard in the contract, at a call, at a return and in a loop
    auto nullp = [](Kit& k, StructType* P)
    {
        return k.nul(k.ptr(k.st(P)));
    };
    auto ownp = [=](Kit& k, Function* f, StructType* P, int amount)
    {
        return k.lor(k.eq(fx(k, f, 0), nullp(k, P)), RO(k, f, amount));
    };
    // void g(struct P *p, int n) requires(p == NULL || owns_range(p, n)) ensures(the same) { if (p != NULL && n > 1) { p[1].x = 7; } [check(false);] }
    const auto own_g = [=](Kit& k, StructType* P, bool vac_) -> Function*
    {
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(g, ownp(k, g, P, 0));
        k.ens(g, ownp(k, g, P, 0));
        std::vector<const Stmt*> bs {k.iff(k.land(k.ne(k.pv(g, 0), nullp(k, P)), k.gt(k.pv(g, 1), N(k, "1"))),
                                           k.blk({k.store(pplace(k, k.pv(g, 0), "1", 0), N(k, "7"))}))};
        if (vac_)
        {
            bs.push_back(k.chk(k.no()));
        }
        g->body = k.blkc(bs);
        k.done(g);
        return g;
    };
    lc("e1e2c1_owning_write",
       "owning(n) callee: p[1].x written under p != NULL && n > 1, the guarded range holds the permission",
       [=](Kit& k)
       {
           own_g(k, mk_p(k), false);
       });
    lc("e1e2c1_owning_write_vacuity",
       "the same with check(false) at the end of the body: it fails (the body is reachable)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* g {own_g(k, P, true)};
           k.fail("check", g->body->stmts.back()->loc);
       });
    lc("e1e2c1_owning_write_unguarded_fail",
       "the write p[1].x without p != NULL in its condition: on the null path the guarded atom holds nothing (null-dereference at the store, once)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(g, ownp(k, g, P, 0));
           k.ens(g, ownp(k, g, P, 0));
           const Stmt* w {k.store(pplace(k, k.pv(g, 0), "1", 0), N(k, "7"))};
           g->body = k.blkc({k.iff(k.gt(k.pv(g, 1), N(k, "1")), k.blk({w}))});
           k.done(g);
           // the null path reaches the store with the guarded atom holding nothing: the null row fails and ends the path, so it is the one error
           k.fail("null-dereference", w->loc);
       });
    // void f(void) { [struct P v[<len>] = {0}; g(&v[0], <n>) | g(NULL, 5)]; [check(false);] }
    const auto own_caller = [=](Kit& k, int mode, unsigned len, const char* n) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {own_g(k, P, false)};
        Function* f {k.fn("f", k.vd)};
        if (mode == 0)
        {
            const Expr* c {k.call(g, {nullp(k, P), N(k, "5")})};
            f->body = k.blkc({k.es(c)});
            k.done(f);
            return c->loc;
        }
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), len), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* c {k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, n)})};
        // after the call the array is still held: v[2].y is read (its flag is in the callee's ensures)
        Var* r {k.local(f, "r", k.i32)};
        std::vector<const Stmt*> bs {k.decl(v, nullptr), k.es(c), k.decl(r, k.deref(pplace(k, k.addr(v), "2", 1)))};
        const Stmt* last {mode == 2 ? k.chk(k.no()) : nullptr};
        if (last != nullptr)
        {
            bs.push_back(last);
        }
        f->body = k.blkc(bs);
        k.done(f);
        return mode == 2 ? last->loc : c->loc;
    };
    lc("e1e2c1_owning_caller_null", "g(NULL, 5): the guard of the callee's atom is false, no row of the range applies",
       [=](Kit& k)
       {
           own_caller(k, 0, 0U, "0");
       });
    lc("e1e2c1_owning_caller_array",
       "struct P v[3] = {0}; g(&v[0], 3): the range holds, v[2].y is still readable after the call",
       [=](Kit& k)
       {
           own_caller(k, 1, 3U, "3");
       });
    lc("e1e2c1_owning_caller_array_vacuity", "the same with check(false) after it: it fails",
       [=](Kit& k)
       {
           k.fail("check", own_caller(k, 2, 3U, "3"));
       });
    lc("e1e2c1_owning_caller_too_long_fail",
       "struct P v[3] = {0}; g(&v[0], 4): the callee's guard holds, the shape row of the range fails alone (permission at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", own_caller(k, 1, 3U, "4"), 12U, 0U);
       });
    // void f(struct P *p, int n) requires(p == NULL || owns_range(p, n)) ensures(the same) { g(p, n [+ 1]); }
    const auto own_forward = [=](Kit& k, bool more) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {own_g(k, P, false)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(ownp(k, f, P, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, ownp(k, f, P, 0));
        const Expr* c {k.call(g, {k.pv(f, 0), more ? k.add(k.pv(f, 1), N(k, "1")) : k.pv(f, 1)})};
        f->body = k.blkc({k.es(c)});
        k.done(f);
        return c->loc;
    };
    lc("e1e2c1_owning_forward",
       "f forwards its owning(n) parameter to g: the call rows and the return rows under the guard",
       [=](Kit& k)
       {
           own_forward(k, false);
       });
    lc("e1e2c1_owning_forward_one_more_fail",
       "f holds p[0..n) (n < 100) and calls g(p, n + 1): on the non-null path the shape row, the length-facts row and the marked row of slot n fail, three rows of one cause (permission at the call)",
       [=](Kit& k)
       {
           const SrcLoc at {own_forward(k, true)};
           // of the call's 12 rows: the shape row (0), the length facts of slot n (4) and the marked value row of member x (5) fail; the first marked row ends its path
           k.fail_only("permission", at, 12U, 0U);
           k.fail_only("permission", at, 12U, 4U);
           k.fail_only("permission", at, 12U, 5U);
       });
    // void fillg(struct P *p, int n) requires((p == NULL || owns_range(p, n)) && n < 100) ensures(the same)
    //   { if (p != NULL) { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i].x = i; i++; } } [check(false);] }
    const auto own_loop = [=](Kit& k, bool vac_) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("fillg", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(ownp(k, f, P, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, ownp(k, f, P, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                                k.blkc({k.store(bat(k, f, 0, k.v(i), 0), k.v(i)), inc(k, i)}))};
        std::vector<const Stmt*> bs {k.iff(k.ne(k.pv(f, 0), nullp(k, P)), k.blk({k.decl(i, N(k, "0")), loop}))};
        const Stmt* c {vac_ ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        f->body = k.blkc(bs);
        k.done(f);
        return c;
    };
    lc("e1e2c1_owning_loop",
       "the fill loop under p != NULL: the range places carry the guard, the null path skips the loop",
       [=](Kit& k)
       {
           own_loop(k, false);
       });
    lc("e1e2c1_owning_loop_vacuity", "the same with check(false) after the if: it fails",
       [=](Kit& k)
       {
           vac(k, own_loop(k, true));
       });
    // ---- the markers (record 0060 item 12, per leaf path) ----
    // void fill(struct P *p, int n) [must_init | ref_to_uninit] requires(owns_range(p, n) && n == 2) ensures(owns_range(p, n))
    //   { writes of p[0].x, p[0].y, p[1].x, p[1].y except the `skip`th (0 to 3), [int r = p[0].x first when `read_first`]; [check(false);] }
    const auto mfill = [=](Kit& k, StructType* P, InitMarker m, int skip, const Expr** read_at, bool vac_) -> Function*
    {
        Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const bool read_first {read_at != nullptr};
        const Expr* first_read {read_first ? k.deref(pplace(k, k.pv(f, 0), "0", 0)) : nullptr};
        if (read_first)
        {
            *read_at = first_read;
        }
        f->params[0U].init = m;
        k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
        k.ens(f, RO(k, f, 0));
        std::vector<const Stmt*> bs;
        if (read_first)
        {
            bs.push_back(k.decl(k.local(f, "r", k.i32), first_read));
        }
        for (int w {0}; w < 4; ++w)
        {
            if (w != skip)
            {
                bs.push_back(k.store(pplace(k, k.pv(f, 0), w < 2 ? "0" : "1", w % 2), N(k, "1")));
            }
        }
        if (vac_)
        {
            bs.push_back(k.chk(k.no()));
        }
        f->body = k.blkc(bs);
        k.done(f);
        return f;
    };
    lc("e1e2c1_mark_must_init_fill",
       "must_init filler of two elements writes every member: the flag row of each leaf holds at the return",
       [=](Kit& k)
       {
           mfill(k, mk_p(k), InitMarker::MustInit, -1, nullptr, false);
       });
    lc("e1e2c1_mark_must_init_fill_vacuity",
       "the same with check(false) at the end: it fails (the body is reachable, the requires has no flag)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {mfill(k, P, InitMarker::MustInit, -1, nullptr, true)};
           k.fail("check", f->body->stmts.back()->loc);
       });
    // the twins: the filler misses one member of one element (leaf y of element 1, then leaf x of element 0): exactly one error at the return, of that leaf's flag row
    lc("e1e2c1_mark_must_init_misses_y_of_one_fail",
       "the filler leaves p[1].y unwritten: the flag row of leaf y fails once at the return (postcondition), the row of leaf x holds",
       [=](Kit& k)
       {
           Function* f {mfill(k, mk_p(k), InitMarker::MustInit, 3, nullptr, false)};
           k.fail("postcondition", f->end_loc, 1U, "", "member `y`");
       });
    lc("e1e2c1_mark_must_init_misses_x_of_zero_fail",
       "the filler leaves p[0].x unwritten: the flag row of leaf x fails once at the return (postcondition)",
       [=](Kit& k)
       {
           Function* f {mfill(k, mk_p(k), InitMarker::MustInit, 0, nullptr, false)};
           k.fail("postcondition", f->end_loc, 1U, "", "member `x`");
       });
    lc("e1e2c1_mark_ref_to_uninit_writes_some",
       "ref_to_uninit writer of three of the four leaves: no flag is promised, the kept-flag rows hold",
       [=](Kit& k)
       {
           mfill(k, mk_p(k), InitMarker::RefToUninit, 2, nullptr, false);
       });
    lc("e1e2c1_mark_ref_to_uninit_reads_first_fail",
       "ref_to_uninit reader of p[0].x before any write: the requires has no flag of any leaf (uninitialized at the read)",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           mfill(k, mk_p(k), InitMarker::RefToUninit, -1, &rd, false);
           k.fail("uninitialized", rd->loc);
       });
    lc("e1e2c1_mark_ref_to_uninit_reads_other_leaf_fail",
       "ref_to_uninit: p[0].y is written, then p[0].x is read (the flag of leaf x of element 0 is unknown): uninitialized once, at that read",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
           k.ens(f, RO(k, f, 0));
           const Stmt* w {k.store(pplace(k, k.pv(f, 0), "0", 1), N(k, "1"))};
           const Expr* rd {k.deref(pplace(k, k.pv(f, 0), "0", 0))};
           f->body = k.blkc({w, k.decl(k.local(f, "r", k.i32), rd)});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    lc("e1e2c1_mark_ref_to_uninit_reads_other_element_fail",
       "ref_to_uninit: p[0].x is written, then p[1].x is read (the flags are per slot: element 1 has none): uninitialized once, at that read",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
           k.ens(f, RO(k, f, 0));
           const Stmt* w {k.store(pplace(k, k.pv(f, 0), "0", 0), N(k, "1"))};
           const Expr* rd {k.deref(pplace(k, k.pv(f, 0), "1", 0))};
           f->body = k.blkc({w, k.decl(k.local(f, "r", k.i32), rd)});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    // void fill(struct P *p, int n) [must_init | ref_to_uninit] requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i].x = i; p[i].y = i; i++; } }
    const auto mloop = [=](Kit& k, InitMarker m) -> Function*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        f->params[0U].init = m;
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* loop {k.whl(
            k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
            k.blkc({k.store(bat(k, f, 0, k.v(i), 0), k.v(i)), k.store(bat(k, f, 0, k.v(i), 1), k.v(i)), inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return f;
    };
    lc("e1e2c1_mark_ref_loop_fill",
       "ref_to_uninit over struct P: the loop writes p[i].x and p[i].y, the range places of both leaves carry the permissions and keep the flags that were set",
       [=](Kit& k)
       {
           mloop(k, InitMarker::RefToUninit);
       });
    lc("e1e2c1_mark_must_init_loop_pinned_fail",
       "a LISTED FALSE ALARM: the same loop under must_init: the flags the loop sets are stated by no automatic invariant (a quantified invariant over the members is Part F's), so the flag row of each leaf fails at the return (postcondition, x and y)",
       [=](Kit& k)
       {
           Function* f {mloop(k, InitMarker::MustInit)};
           k.fail("postcondition", f->end_loc, 2U);
       });
    // void outer(struct P *p, int n) [must_init] requires(owns_range(p, n) && n == 2) ensures(owns_range(p, n)) { fill(p, n); }
    const auto mouter = [=](Kit& k, InitMarker callee_marker, InitMarker outer_marker) -> Function*
    {
        StructType* P {mk_p(k)};
        Function* callee {mfill(k, P, callee_marker, -1, nullptr, false)};
        Function* f {k.fn("outer", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        f->params[0U].init = outer_marker;
        k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
        k.ens(f, RO(k, f, 0));
        f->body = k.blkc({k.es(k.call(callee, {k.pv(f, 0), k.pv(f, 1)}))});
        k.done(f);
        return f;
    };
    lc("e1e2c1_mark_must_init_forward",
       "a must_init forwarder: the callee's ensures gives the flag of every leaf, so the return rows hold",
       [=](Kit& k)
       {
           mouter(k, InitMarker::MustInit, InitMarker::MustInit);
       });
    lc("e1e2c1_mark_must_init_forward_ref_fail",
       "a must_init forwarder over a ref_to_uninit callee that writes everything: no flag is promised, so the flag row of each leaf fails at the return (two errors: x and y)",
       [=](Kit& k)
       {
           Function* f {mouter(k, InitMarker::RefToUninit, InitMarker::MustInit)};
           k.fail("postcondition", f->end_loc, 2U);
       });
    lc("e1e2c1_mark_ref_forward",
       "a ref_to_uninit forwarder over a must_init callee: the requires claims no flag, the call has no flag row, the kept-flag rows hold",
       [=](Kit& k)
       {
           mouter(k, InitMarker::MustInit, InitMarker::RefToUninit);
       });
    // void f(void) { struct P v[2] [= {0}]; fill(&v[0], 2); int r = v[1].y; [check(false);] }
    const auto mcaller = [=](Kit& k, InitMarker m, bool zero, bool vac_) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mfill(k, P, m, -1, nullptr, false)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 2U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        Var* r {k.local(f, "r", k.i32)};
        const Expr* c {k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "2")})};
        const Expr* rd {k.deref(pplace(k, k.addr(v), "1", 1))};
        std::vector<const Stmt*> bs {k.decl(v, nullptr), k.es(c), k.decl(r, rd)};
        const Stmt* ck {vac_ ? k.chk(k.no()) : nullptr};
        if (ck != nullptr)
        {
            bs.push_back(ck);
        }
        f->body = k.blkc(bs);
        k.done(f);
        return vac_ ? ck->loc : rd->loc;
    };
    lc("e1e2c1_mark_must_init_caller",
       "struct P v[2] with no value; fill(&v[0], 2) is must_init: no flag row at the call, and v[1].y has its flag afterwards",
       [=](Kit& k)
       {
           mcaller(k, InitMarker::MustInit, false, false);
       });
    lc("e1e2c1_mark_must_init_caller_vacuity", "the same with check(false) after the read: it fails",
       [=](Kit& k)
       {
           k.fail("check", mcaller(k, InitMarker::MustInit, false, true));
       });
    lc("e1e2c1_mark_ref_caller_unset_fail",
       "struct P v[2] with no value; the ref_to_uninit callee writes all four members but promises no flag: the read of v[1].y fails (uninitialized)",
       [=](Kit& k)
       {
           k.fail("uninitialized", mcaller(k, InitMarker::RefToUninit, false, false));
       });
    lc("e1e2c1_mark_ref_caller_kept",
       "struct P v[2] = {0}; the ref_to_uninit callee keeps every flag that was set: v[1].y is readable",
       [=](Kit& k)
       {
           mcaller(k, InitMarker::RefToUninit, true, false);
       });
    lc("e1e2c1_mark_ref_caller_kept_vacuity", "the same with check(false) after the read: it fails",
       [=](Kit& k)
       {
           k.fail("check", mcaller(k, InitMarker::RefToUninit, true, true));
       });
    // a leaf of depth 2: struct Q { int a; struct T { int b; int c; } t; }; the filler misses p[1].t.c
    lc("e1e2c1_mark_must_init_nested_misses_fail",
       "must_init over struct Q with a nested struct: the filler writes every leaf of both elements but p[1].t.c: one flag row fails (postcondition, member t.c)",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
           StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::MustInit;
           k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
           k.ens(f, RO(k, f, 0));
           auto leaf = [&](const char* e, int m1, int m2)
           {
               const Expr* x {k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, e), k.at()), m1, k.at())};
               return m2 < 0 ? x : k.b.member_addr(x, m2, k.at());
           };
           f->body = k.blkc({k.store(leaf("0", 0, -1), N(k, "1")), k.store(leaf("0", 1, 0), N(k, "1")),
                             k.store(leaf("0", 1, 1), N(k, "1")), k.store(leaf("1", 0, -1), N(k, "1")),
                             k.store(leaf("1", 1, 0), N(k, "1"))});
           k.done(f);
           k.fail("postcondition", f->end_loc, 1U, "", "t.c");
       });
}

} // namespace
