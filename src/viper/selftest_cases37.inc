// (included by selftest.cpp) M4.5e1e2 continuation, Part E (record 0064 items 2 to 6, 10 to 13): arrays of arrays.  Slice 1 of the lowering: the typing of an element address over a row
// (`Ptr(Array)` is a row PARAMETER's type and nothing else), the ONE enumeration of the cells of a structure value, and the automatic local matrix of scalars in code.  The names of the
// cases start with `e1e2e_`.
namespace {

// int[3]
Type e1e2e_row(Kit& k)
{
    return k.prog.array_of(k.i32, 3U);
}
// int[2][3]
Type e1e2e_matrix(Kit& k)
{
    return k.prog.array_of(e1e2e_row(k), 2U);
}

void define_part_e_pointer_to_array_rejections()
{
    // `Ptr(Array)` is the type of a row PARAMETER (`T (*a)[M]`) and of nothing else (record 0064 items 4 and 13: refused as a local, a member, a result, a cell, a cast, a temporary).  Slice 1
    // accepts none of them (a row parameter is slice 2's); the local and the parameter cases are the older `arr_rej_pointer_to_array_*`.
    rc("e1e2e_rej_pointer_to_row_member", "a pointer to int[3]",
       [=](Kit& k)
       {
           // struct S { int (*q)[3]; };
           k.strukt("S", {{"q", k.ptr(e1e2e_row(k))}});
       });
    rc("e1e2e_rej_pointer_to_row_result", "the return type of 'f' is a pointer to int[3]",
       [=](Kit& k)
       {
           // int (*f(void))[3];
           Function* f {k.fn("f", k.ptr(e1e2e_row(k)))};
           k.trusted(f);
       });
    rc("e1e2e_rej_pointer_to_row_cell", "variable 'c' is a pointer to int[3]",
       [=](Kit& k)
       {
           // int (*c)[3]; ... &c: a cell that holds a pointer to a row
           Function* f {k.fn("f", k.vd)};
           Var* c {k.cell(f, "c", k.ptr(e1e2e_row(k)))};
           f->body = k.blkc({k.decl(c, nullptr)});
           k.done(f);
       });
    rc("e1e2e_rej_pointer_to_pointer_to_row_parameter", "parameter 1 of 'f' is a pointer to int[3]",
       [=](Kit& k)
       {
           // void f(int (**pp)[3]) {}: a pointer to a cell that holds a row pointer
           Function* f {k.fn("f", k.vd, {{"pp", k.ptr(k.ptr(e1e2e_row(k)))}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_pointer_to_row_cast", "a pointer to int[3]",
       [=](Kit& k)
       {
           // (int (*)[3])p: a conversion to a pointer to a row (the conversion node and its type are both refused)
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.i32)}})};
           f->body = k.blk({k.es(k.cv(k.ptr(e1e2e_row(k)), k.pv(f, 0)))});
           k.done(f);
       });
    rc("e1e2e_rej_pointer_to_row_temporary", "an expression of type",
       [=](Kit& k)
       {
           // c ? (int (*)[3])0 : (int (*)[3])0 as a statement's value: a temporary of type pointer to a row
           Function* f {k.fn("f", k.vd, {{"c", k.i32}})};
           const Type pr {k.ptr(e1e2e_row(k))};
           f->body = k.blk({k.es(k.cond(k.tb(k.pv(f, 0)), k.nul(pr), k.nul(pr)))});
           k.done(f);
       });
}

void define_part_e_value_rejections()
{
    // record 0064 item 12 (Part E, slice 4a): a structure value of depth 2 or more is the tuple of vir::value_cells and is accepted up to 7 cells reached through an index step (struct U { int
    // m[2][3]; int n; } is 6); what stays refused: a value past the limit (M4.5f3), a row of a matrix member of a value (question 364), an array member read as a value, and the positions
    // 0044 item 2 never allowed.  The types: a matrix member (6 cells, and 8 past the limit), a member array of structures whose element has an array member (6, and 10 past it).
    auto matrix_struct = [](Kit& k)
    {
        return k.strukt("U", {{"m", e1e2e_matrix(k)}, {"n", k.i32}});
    };
    auto wide_struct = [](Kit& k)
    {
        return k.strukt("U8", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 4U), 2U)}, {"n", k.i32}});
    };
    auto nested_struct = [](Kit& k)
    {
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 2U)}, {"n", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}});
    };
    auto wide_nested_struct = [](Kit& k)
    {
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 4U)}, {"n", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}});
    };
    rc("e1e2e_rej_value_parameter_matrix_member_beyond_the_limit", "array elements passed by value",
       [=](Kit& k)
       {
           // void f(struct U8 u) {}  with struct U8 { int m[2][4]; int n; }: 8 cells reached through an index step
           Function* f {k.fn("f", k.vd, {{"u", k.st(wide_struct(k))}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_value_parameter_nested_array_member_beyond_the_limit", "array elements passed by value",
       [=](Kit& k)
       {
           // void f(struct T t) {}  with struct T { struct W v[2]; } and struct W { int a[4]; int n; }: 10 cells (the record's own example)
           Function* f {k.fn("f", k.vd, {{"t", k.st(wide_nested_struct(k))}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_value_result_matrix_member_beyond_the_limit", "array elements passed by value",
       [=](Kit& k)
       {
           // struct U8 f(void);
           Function* f {k.fn("f", k.st(wide_struct(k)))};
           k.trusted(f);
       });
    rc("e1e2e_rej_value_result_nested_array_member_beyond_the_limit", "array elements passed by value",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.st(wide_nested_struct(k)))};
           k.trusted(f);
       });
    rc("e1e2e_rej_value_aggregate_matrix_member_beyond_the_limit", "holds more than 7",
       [=](Kit& k)
       {
           // (struct U8){...} as a value: an Aggregate of 9 cells
           StructType* U {wide_struct(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", U)};
           const Expr* lit {k.b.aggregate(k.st(U), {k.num(k.i32, "0")}, k.at())};
           f->body = k.blkc({k.decl(u, nullptr), k.store(k.addr(u), lit)});
           k.done(f);
       });
    rc("e1e2e_rej_value_load_nested_array_member", "in a position decision 0044 item 2 does not allow",
       [=](Kit& k)
       {
           // struct T t; (void)t;  as a read of the whole object
           StructType* T {nested_struct(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           f->body = k.blkc({k.decl(t, nullptr), k.es(k.deref(k.addr(t)))});
           k.done(f);
       });
    rc("e1e2e_rej_value_store_matrix_member", "in a position decision 0044 item 2 does not allow",
       [=](Kit& k)
       {
           // struct U u, w; u = w;  as the store of a whole value (the encoder writes a copy cell by cell, never as a value)
           StructType* U {matrix_struct(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", U)};
           Var* w {k.scell(f, "w", U)};
           f->body = k.blkc({k.decl(u, nullptr), k.decl(w, nullptr), k.store(k.addr(u), k.deref(k.addr(w)))});
           k.done(f);
       });
    rc("e1e2e_rej_value_aggregate_matrix_member_leaf_count",
       "an aggregate of struct U with 2 arguments, 7 leaves expected",
       [=](Kit& k)
       {
           // (struct U){...} with two arguments: the value has 7 cells
           StructType* U {matrix_struct(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", U)};
           const Expr* lit {k.b.aggregate(k.st(U), {k.num(k.i32, "0"), k.num(k.i32, "0")}, k.at())};
           f->body = k.blkc({k.decl(u, nullptr), k.store(k.addr(u), lit)});
           k.done(f);
       });
    // question 364: a row of a matrix member of a structure value is not a value, so `v.m[i][j]` of a by-value parameter in a contract is refused
    rc("e1e2e_rej_value_contract_reads_a_row_of_a_matrix_member", "is a row, and an array is never a value",
       [=](Kit& k)
       {
           // void f(struct U u) requires(u.m[1] == ...) {}
           StructType* U {matrix_struct(k)};
           Function* f {k.fn("f", k.vd, {{"u", k.st(U)}})};
           f->mark_address_taken(f->param_var(0));
           k.req(f, k.eq(k.b.elem_value(k.F(f, 0), 0, k.num(k.i32, "1"), k.at()), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_value_member_array_read_as_a_value", "an array is never a value",
       [=](Kit& k)
       {
           // void f(struct U u) requires(u.m == ...) {}: the whole matrix member of a value
           StructType* U {matrix_struct(k)};
           Function* f {k.fn("f", k.vd, {{"u", k.st(U)}})};
           f->mark_address_taken(f->param_var(0));
           k.req(f, k.eq(k.b.field(k.F(f, 0), 0, k.at()), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
}

// ---- decision 0064 items 3, 5, 6 and 13 (Part E, slice 1): the automatic local matrix of scalars in code ----
void define_part_e_local_matrix_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // int m[2][3];  (zero: its initializer leaves cells out, C's zero fill)
    auto mat = [](Kit& k, Function* f, bool zero)
    {
        Var* m {f->new_local("m", e1e2e_matrix(k), k.at())};
        f->mark_address_taken(m);
        m->zero_filled = zero;
        return m;
    };
    auto row = [](Kit& k, Var* m, const Expr* i)
    {
        return k.b.elem_addr(k.addr(m), i, k.at());
    };
    // the address of m[i][j]; as a place and as a read
    auto place = [=](Kit& k, Var* m, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(row(k, m, i), j, k.at());
    };
    auto read = [=](Kit& k, Var* m, const Expr* i, const Expr* j)
    {
        return k.deref(place(k, m, i, j));
    };

    // int f(int i, int j) requires(0 <= i <= hi_i && 0 <= j <= hi_j) ensures(r == 7) { int m[2][3]; m[i][j] = 3; return m[i][j] + 4; }
    const auto rw = [=](Kit& k, const char* hi_i, const char* hi_j) -> const Stmt*
    {
        Function* f {k.fn("f", k.i32, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", hi_i), k.range(k.F(f, 1), k.i32, "0", hi_j)));
        Var* m {mat(k, f, false)};
        const Stmt* w {k.store(place(k, m, k.pv(f, 0), k.pv(f, 1)), N(k, "3"))};
        f->body = k.blkc({k.decl(m, nullptr), w, k.ret(k.add(read(k, m, k.pv(f, 0), k.pv(f, 1)), N(k, "4")))});
        k.ens(f, k.eq(k.R(f), N(k, "7")));
        k.done(f);
        return w;
    };
    lc("e1e2e_matrix_rw",
       "m[i][j] written and read for symbolic indexes of a local matrix: both bound rows, then the cell's rows",
       [=](Kit& k)
       {
           rw(k, "1", "2");
       });
    lc("e1e2e_matrix_first_index_high_fail",
       "m[i][j] with i == 2: out-of-bounds at the store, the first index, before the cell's rows",
       [=](Kit& k)
       {
           // the two bound rows of the access are at the store's line; the first one fails
           k.fail_only("out-of-bounds", rw(k, "2", "2")->loc, 2U, 0U);
       });
    lc("e1e2e_matrix_second_index_high_fail",
       "m[i][j] with j == 3: out-of-bounds at the store, the second index against the row's own length",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", rw(k, "1", "3")->loc, 2U, 1U);
       });
    // int f(void) { int m[2][3] = {0}; check(m[1][2] == want); return 0; }
    const auto zero = [=](Kit& k, const char* want) -> SrcLoc
    {
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        const Stmt* c {k.chk(k.eq(read(k, m, N(k, "1"), N(k, "2")), N(k, want)))};
        f->body = k.blkc({k.decl(m, nullptr), c, k.ret(N(k, "0"))});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_matrix_zero_fill", "a matrix with the zero fill: every cell of every row is zero and set",
       [=](Kit& k)
       {
           zero(k, "0");
       });
    lc("e1e2e_matrix_zero_fill_wrong_fail", "the zero fill: the check that m[1][2] is 1 fails",
       [=](Kit& k)
       {
           k.fail("check", zero(k, "1"));
       });
    // int f(void) { int m[2][3] = {{1}, {0, 7}}; ... } as the stores the encoder emits after the zero fill: m[0][0] = 1; m[1][1] = 7;  check(m[<ri>][<rj>] == want)
    const auto init = [=](Kit& k, const char* ri, const char* rj, const char* want) -> SrcLoc
    {
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        const Stmt* c {k.chk(k.eq(read(k, m, N(k, ri), N(k, rj)), N(k, want)))};
        f->body = k.blkc({k.decl(m, nullptr), k.store(place(k, m, N(k, "0"), N(k, "0")), N(k, "1")),
                          k.store(place(k, m, N(k, "1"), N(k, "1")), N(k, "7")), c, k.ret(N(k, "0"))});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_matrix_initializer_cells_are_distinct",
       "the cells the list writes hold their values and the rest is zero: m[0][0] is 1, m[1][1] is 7, m[0][1] is 0, m[1][0] is 0",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, true)};
           f->body = k.blkc({k.decl(m, nullptr), k.store(place(k, m, N(k, "0"), N(k, "0")), N(k, "1")),
                             k.store(place(k, m, N(k, "1"), N(k, "1")), N(k, "7")),
                             k.chk(k.land(k.land(k.eq(read(k, m, N(k, "0"), N(k, "0")), N(k, "1")),
                                                 k.eq(read(k, m, N(k, "1"), N(k, "1")), N(k, "7"))),
                                          k.land(k.eq(read(k, m, N(k, "0"), N(k, "1")), N(k, "0")),
                                                 k.eq(read(k, m, N(k, "1"), N(k, "0")), N(k, "0"))))),
                             k.ret(N(k, "0"))});
           k.done(f);
       });
    lc("e1e2e_matrix_initializer_wrong_cell_fail",
       "the same list: the check that m[1][0] is 7 (the value of m[1][1], the same second index is not the same cell) fails",
       [=](Kit& k)
       {
           k.fail("check", init(k, "1", "0", "7"));
       });
    lc("e1e2e_matrix_initializer_wrong_row_fail",
       "the same list: the check that m[0][1] is 7 (the value of m[1][1], the same first index is not the same cell) fails",
       [=](Kit& k)
       {
           k.fail("check", init(k, "0", "1", "7"));
       });
    lc("e1e2e_matrix_uninitialized_read_fail",
       "int m[2][3]; m[0][1] = 5; return m[1][1]: the cell m[1][1] has no value: uninitialized at the read",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, false)};
           const Expr* rd {read(k, m, N(k, "1"), N(k, "1"))};
           f->body = k.blkc({k.decl(m, nullptr), k.store(place(k, m, N(k, "0"), N(k, "1")), N(k, "5")), k.ret(rd)});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    // &m[i][j] as a value: the first index strictly inside (the row's slot must exist), the second one past the end at most.  int f(void) { int m[2][3]; int *q = &m[<i>][<j>]; return 0; }
    const auto formed = [=](Kit& k, const char* i, const char* j) -> SrcLoc
    {
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        Var* q {k.local(f, "q", k.ptr(k.i32))};
        const Expr* addr {place(k, m, N(k, i), N(k, j))};
        f->body = k.blkc({k.decl(m, nullptr), k.decl(q, addr), k.ret(N(k, "0"))});
        k.done(f);
        return addr->loc;
    };
    lc("e1e2e_matrix_address_one_past_the_row",
       "&m[1][3] is formed: one past the end of the row is allowed at the last index",
       [=](Kit& k)
       {
           formed(k, "1", "3");
       });
    lc("e1e2e_matrix_address_past_the_row_fail",
       "&m[1][4] is beyond one past the end of m[1]: out-of-bounds where it is formed",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", formed(k, "1", "4"), 2U, 1U);
       });
    lc("e1e2e_matrix_address_past_the_matrix_fail",
       "&m[2][0] needs the row slot 2 to exist: the first index is strictly inside (out-of-bounds where it is formed)",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", formed(k, "2", "0"), 2U, 0U);
       });
    // void fill(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n));  (declared, defined elsewhere)
    // int f(void) { int m[2][3] = {0}; fill(m[1], <n>); check(m[0][0] == 0); return m[1][0] - m[1][0]; }
    const auto row_call = [=](Kit& k, const char* n) -> const Expr*
    {
        Function* g {k.fn("fill", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
        k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
        k.ens(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
        k.trusted(g);
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        const Expr* callx {k.call(g, {place(k, m, N(k, "1"), N(k, "0")), N(k, n)})};
        const Stmt* call {k.es(callx)};
        f->body = k.blkc({k.decl(m, nullptr), call, k.chk(k.eq(read(k, m, N(k, "0"), N(k, "0")), N(k, "0"))),
                          k.ret(k.sub(read(k, m, N(k, "1"), N(k, "2")), read(k, m, N(k, "1"), N(k, "2"))))});
        k.done(f);
        return callx;
    };
    lc("e1e2e_matrix_row_to_a_range_callee",
       "the row m[1] (its decay &m[1][0]) given to a callee that owns p[0..3) and gives it back: the permission is taken out of the matrix's chunk and returned, the other row is unchanged",
       [=](Kit& k)
       {
           row_call(k, "3");
       });
    lc("e1e2e_matrix_row_range_past_the_row_fail",
       "the row m[1] given to a callee that owns p[0..4): the row has three cells, so the call is a permission error",
       [=](Kit& k)
       {
           k.fail_only("permission", row_call(k, "4")->loc, 8U, 0U);
       });
    // the contracts are satisfiable (a vacuity twin per trusted method): the state after the allocation and after the zero fill is reachable, so `check(false)` there FAILS
    lc("e1e2e_matrix_alloc_vacuity_fail",
       "int m[2][3]; m[1][2] = 1; check(false): the allocation's contract is satisfiable (the check is reached and fails)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* m {mat(k, f, false)};
           const Stmt* c {k.chk(k.no())};
           f->body = k.blkc({k.decl(m, nullptr), k.store(place(k, m, N(k, "1"), N(k, "2")), N(k, "1")), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    lc("e1e2e_matrix_zero_vacuity_fail",
       "int m[2][3] = {0}; check(m[1][2] == 0); check(false): the fill's contract is satisfiable (the last check is reached and fails)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* m {mat(k, f, true)};
           const Stmt* c {k.chk(k.no())};
           f->body = k.blkc({k.decl(m, nullptr), k.chk(k.eq(read(k, m, N(k, "1"), N(k, "2")), N(k, "0"))), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    // void keep(int *p, int n) requires(owns_range(p, n));  void f(void) { int m[2][3] = {0}; keep(m[1], 3); }: the callee keeps the row's cells, so the scope exit is a lifetime error
    lc("e1e2e_matrix_row_kept_fail",
       "the row m[1] given to a callee that keeps its cells: lifetime at the first marked scope-exit row",
       [=](Kit& k)
       {
           Function* g {k.fn("keep", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.trusted(g);
           Function* f {k.fn("f", k.vd)};
           Var* m {mat(k, f, true)};
           const Stmt* bs {
               k.blkc({k.decl(m, nullptr), k.es(k.call(g, {place(k, m, N(k, "1"), N(k, "0")), N(k, "3")}))})};
           f->body = bs;
           k.done(f);
           k.fail_only("lifetime", bs->end_loc, 3U, 0U);
       });
    // ---- the other cell types of a matrix (each cell type has its own trusted `alloc_arr_row_<t>_3` and `zero_arr_row_<t>_3`): bool, an enumeration and a pointer, the zero branch of each ----
    // <cell> m[2][3] = {0}; check(<test>); (<test> reads m[1][2])
    const auto typed = [=](Kit& k, const Type& cell, int test, bool vacuous) -> SrcLoc
    {
        Function* f {k.fn("f", k.vd)};
        Var* m {f->new_local("m", k.prog.array_of(k.prog.array_of(cell, 3U), 2U), k.at())};
        f->mark_address_taken(m);
        m->zero_filled = true;
        const Expr* r {k.deref(k.b.elem_addr(k.b.elem_addr(k.addr(m), N(k, "1"), k.at()), N(k, "2"), k.at()))};
        const Expr* cond {nullptr};
        if (cell.is_bool())
        {
            cond = test == 0 ? k.lnot(r) : r; // 0: the zero is false; 1: wrongly true
        }
        else if (cell.is_ptr())
        {
            cond = test == 0 ? k.eq(r, k.nul(cell)) : k.ne(r, k.nul(cell));
        }
        else
        {
            cond = k.eq(r, k.num(cell, test == 0 ? "0" : "1"));
        }
        const Stmt* c {k.chk(vacuous ? k.no() : cond)};
        f->body = vacuous ? k.blkc({k.decl(m, nullptr), k.chk(cond), c}) : k.blkc({k.decl(m, nullptr), c});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_matrix_bool_zero_fill", "bool m[2][3] = {0}: m[1][2] is false (the zero of a bool cell)",
       [=](Kit& k)
       {
           typed(k, k.bl, 0, false);
       });
    lc("e1e2e_matrix_bool_zero_fill_wrong_fail", "bool m[2][3] = {0}: the check that m[1][2] is true fails",
       [=](Kit& k)
       {
           k.fail("check", typed(k, k.bl, 1, false));
       });
    lc("e1e2e_matrix_bool_vacuity_fail",
       "bool m[2][3] = {0}: the contracts of alloc_arr_row_bool_3 and zero_arr_row_bool_3 are satisfiable",
       [=](Kit& k)
       {
           k.fail("check", typed(k, k.bl, 0, true));
       });
    lc("e1e2e_matrix_pointer_zero_fill", "int *m[2][3] = {0}: m[1][2] is null (the zero of a pointer cell)",
       [=](Kit& k)
       {
           typed(k, k.ptr(k.i32), 0, false);
       });
    lc("e1e2e_matrix_pointer_zero_fill_wrong_fail", "int *m[2][3] = {0}: the check that m[1][2] is not null fails",
       [=](Kit& k)
       {
           k.fail("check", typed(k, k.ptr(k.i32), 1, false));
       });
    lc("e1e2e_matrix_pointer_vacuity_fail",
       "int *m[2][3] = {0}: the contracts of the pointer cell's trusted methods are satisfiable",
       [=](Kit& k)
       {
           k.fail("check", typed(k, k.ptr(k.i32), 0, true));
       });
    lc("e1e2e_matrix_enum_zero_fill", "enum E m[2][3] = {0} (E { A = 0, B = 1 }): m[1][2] is 0",
       [=](Kit& k)
       {
           typed(k, k.en("E", 32, false, "0", "1"), 0, false);
       });
    lc("e1e2e_matrix_enum_zero_fill_wrong_fail", "enum E m[2][3] = {0}: the check that m[1][2] is 1 fails",
       [=](Kit& k)
       {
           k.fail("check", typed(k, k.en("E", 32, false, "0", "1"), 1, false));
       });
    lc("e1e2e_matrix_enum_vacuity_fail",
       "enum E m[2][3] = {0}: the contracts of the enumeration cell's trusted methods are satisfiable",
       [=](Kit& k)
       {
           k.fail("check", typed(k, k.en("E", 32, false, "0", "1"), 0, true));
       });
    // ---- a row given to a range callee beside a cell access in one full expression: nothing stands behind the unsequenced rows (item 11) ----
    // int wr(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n) && r == 0);  int f(void) { int m[2][3] = {0}; return wr(m[1], 3) + m[<row>][0]++; }
    const auto row_seq = [=](Kit& k, const char* row_of_the_cell) -> const Expr*
    {
        Function* g {k.fn("wr", k.i32, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
        k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
        k.ens(g, k.land(k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()), k.eq(k.R(g), N(k, "0"))));
        k.trusted(g);
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        const Expr* call {k.call(g, {place(k, m, N(k, "1"), N(k, "0")), N(k, "3")})};
        const Expr* inc {k.b.inc_dec_store_expr(place(k, m, N(k, row_of_the_cell), N(k, "0")), true, true, k.at())};
        const Expr* sum {k.add(call, inc)};
        f->body = k.blkc({k.decl(m, nullptr), k.ret(sum)});
        k.done(f);
        return inc;
    };
    lc("e1e2e_matrix_row_call_beside_another_row",
       "wr(m[1], 3) + m[0][0]++: the call's range is row 1, the cell is in row 0: no unsequenced row fails",
       [=](Kit& k)
       {
           row_seq(k, "0");
       });
    lc("e1e2e_matrix_row_call_beside_a_cell_of_the_row_fail",
       "wr(m[1], 3) + m[1][0]++: the cell is in the row the call owns: unsequenced at the later access",
       [=](Kit& k)
       {
           const Expr* o {row_seq(k, "1")};
           k.fail_only("unsequenced", o->loc, 2U, 0U);
       });
    // a pointer the function obtains from a call is split against the cells in scope (a no-op branch that lets Silicon find a permission): against the first cell of the first row of a local matrix
    // int *get(void) ensures(owns(*r)); int f(void) { int m[2][3] = {0}; int *p = get(); *p = 4; return *p; }
    lc("e1e2e_matrix_split_for_a_pointer_from_a_call",
       "a pointer obtained from a call beside a local matrix: the function verifies and the case split names the first cell of the first row",
       [=](Kit& k)
       {
           Function* g {k.fn("get", k.ptr(k.i32))};
           k.ens(g, k.b.atom(AtomKind::Owns, k.R(g), 0, k.at()));
           k.trusted(g);
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, true)};
           Var* p {k.local(f, "p", k.ptr(k.i32))};
           f->body = k.blkc(
               {k.decl(m, nullptr), k.decl(p, k.call(g, {})), k.store(k.pn(p), N(k, "4")), k.ret(k.deref(k.pn(p)))});
           k.done(f);
           k.text_checks.push_back(
               [](const std::string& vpr) -> std::string
               {
                   return vpr.find("== loc(member(v_m__") != std::string::npos
                              ? ""
                              : "no case split against a cell of the matrix";
               });
       });
    // every way out of the scope exhales what the allocation gave (the two quantifiers of the cells' value and flag and the liveness cell)
    const auto exits_pin = [](const std::string& vpr) -> std::string
    {
        size_t exhales {0U};
        size_t marked {0U};
        size_t alloc {0U};
        bool in_alloc {false};
        std::istringstream in(vpr);
        for (std::string l; std::getline(in, l);)
        {
            if (l.rfind("method alloc_arr_row_int_3", 0U) == 0U)
            {
                in_alloc = true;
                continue;
            }
            if (l.rfind("method ", 0U) == 0U)
            {
                in_alloc = false;
            }
            const bool quant {l.find("forall k: Int, j: Int :: { loc(member(loc(block_of(") != std::string::npos};
            const bool value {l.find(".v_int)") != std::string::npos};
            const bool flag {l.find(".d_int)") != std::string::npos};
            if (in_alloc && l.find("ensures") != std::string::npos && quant && (value || flag) &&
                l.find("==> acc(") != std::string::npos)
            {
                ++alloc;
            }
            if (l.find("exhale forall k: Int, j: Int") != std::string::npos && (value || flag))
            {
                ++exhales;
            }
            if (l.find("assert forall k: Int, j: Int") != std::string::npos && (value || flag))
            {
                ++marked;
            }
        }
        if (alloc != 2U)
        {
            return "the allocation of the matrix gives " + std::to_string(alloc) + " permission quantifiers, not two";
        }
        if (exhales < 2U * 3U || marked != exhales)
        {
            return "at least three ways out of the scope: " + std::to_string(exhales) + " exhales and " +
                   std::to_string(marked) +
                   " marked rows of the cells' value and flag permission, at least 6 and as many marked rows expected";
        }
        return "";
    };
    // int f(int c) { int m[2][3]; if (c != 0) { return 1; } { int w[2][3]; w[0][0] = 1; } return 0; }: the return in the `if`, the end of the inner block, the final return
    lc("e1e2e_matrix_scope_exits_give_what_the_allocation_gave",
       "every way out of a local matrix's scope exhales exactly the allocation's two cell quantifiers",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"c", k.i32}})};
           Var* m {mat(k, f, false)};
           Var* w {mat(k, f, false)};
           const Stmt* inner {k.blkc({k.decl(w, nullptr), k.store(place(k, w, N(k, "0"), N(k, "0")), N(k, "1"))})};
           f->body = k.blkc({k.decl(m, nullptr), k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.ret(N(k, "1"))})), inner,
                             k.ret(N(k, "0"))});
           k.done(f);
           k.text_checks.push_back(exits_pin);
       });
    // ---- sequencing: every index is in the key of a cell (item 11) ----
    // void f(int i, int j, int p, int q) requires(...ranges) { int m[2][3]; (every cell written); m[<li>][<lj>] = m[<ri>][<rj>]++; }
    const auto seq = [=](Kit& k, const char* li, const char* lj, const char* ri, const char* rj, const Expr** out_outer)
    {
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}, {"p", k.i32}, {"q", k.i32}})};
        k.req(f, k.land(k.land(k.range(k.F(f, 0), k.i32, "0", "1"), k.range(k.F(f, 1), k.i32, "0", "2")),
                        k.land(k.range(k.F(f, 2), k.i32, "0", "1"), k.range(k.F(f, 3), k.i32, "0", "2"))));
        Var* m {mat(k, f, true)};
        auto ix = [&](const char* t) -> const Expr*
        {
            const std::string s {t};
            return s == "i"   ? k.pv(f, 0)
                   : s == "j" ? k.pv(f, 1)
                   : s == "p" ? k.pv(f, 2)
                   : s == "q" ? k.pv(f, 3)
                              : N(k, t);
        };
        const Expr* lhs {place(k, m, ix(li), ix(lj))};
        const Expr* rhs {place(k, m, ix(ri), ix(rj))};
        const Expr* inner {k.b.inc_dec_store_expr(rhs, true, true, k.at())};
        const Expr* outer {k.b.store_expr(lhs, inner, k.at())};
        f->body = k.blkc({k.decl(m, nullptr), k.es(outer)});
        k.done(f);
        *out_outer = outer;
    };
    lc("e1e2e_matrix_seq_same_first_index_other_cell", "m[0][1] = m[0][2]++: two cells of one row are two objects",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, "0", "1", "0", "2", &o);
       });
    lc("e1e2e_matrix_seq_same_second_index_other_row",
       "m[0][1] = m[1][1]++: two cells of one column are two objects (a key that ignored the first index would reject it)",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, "0", "1", "1", "1", &o);
       });
    rc("e1e2e_matrix_rej_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, "1", "2", "1", "2", &o);
       });
    lc("e1e2e_matrix_seq_unknown_indexes_fail",
       "m[i][j] = m[p][q]++ with the indexes unknown: the two cells may be one: unsequenced at the later access",
       [=](Kit& k)
       {
           const Expr* o {nullptr};
           seq(k, "i", "j", "p", "q", &o);
           k.fail_only("unsequenced", o->loc, 1U, 0U);
       });
}

// A row parameter's element address read as a flat pointer: refused by check_program (a row parameter is slice 2's) and, lowered anyway, an internal error: the guard that stands until the slice that
// accepts the parameter gives its rows
void define_part_e_row_guard_cases()
{
    // (slice 2a lifted the guard for a row parameter's element in code: the case is now the store's rows, `e1e2e_rowparam_write_without_atom_fail` in define_part_e_row_parameter_code_cases)
    // (slice 2b lifted trap 2 for a row parameter's contract read: the pin e1e2e_ru_row_parameter_contract_read_has_no_key is now `e1e2e_rowparam_contract_reads_keys` in define_part_e_row_parameter_contract_cases)
    rc("e1e2e_rej_row_address_as_a_value", "an expression of type",
       [=](Kit& k)
       {
           // int m[2][3]; (void)&m[1];  the address of a row is no value: it is the base of an element address and nothing else
           Function* f {k.fn("f", k.vd)};
           Var* m {f->new_local("m", e1e2e_matrix(k), k.at())};
           f->mark_address_taken(m);
           f->body = k.blkc({k.decl(m, nullptr), k.es(k.b.elem_addr(k.addr(m), k.num(k.i32, "1"), k.at()))});
           k.done(f);
       });
}

// ---- the refusals of what slice 1 does not lift ----
void define_part_e_local_matrix_rejections()
{
    rc("e1e2e_rej_matrix_depth3_local", "variable 'c' is an array of",
       [=](Kit& k)
       {
           // int c[2][3][4]: depth 3 is slice 4's
           Function* f {k.fn("f", k.vd)};
           Var* c {k.cell(f, "c", k.prog.array_of(k.prog.array_of(k.prog.array_of(k.i32, 4U), 3U), 2U))};
           f->body = k.blkc({k.decl(c, nullptr)});
           k.done(f);
       });
    rc("e1e2e_rej_matrix_global_depth3", "depth 3, an array of arrays of arrays",
       [=](Kit& k)
       {
           // int g[2][2][3]; at file scope: depth 3 is slice 4's
           k.prog.add_global("g", k.prog.array_of(e1e2e_matrix(k), 2U), k.at(), nullptr);
       });
    rc("e1e2e_rej_matrix_member_depth3", "the member 'm' of the struct 'struct U' is an array of int[3][3]",
       [=](Kit& k)
       {
           // struct U { int m[2][3][3]; }: depth 3 is slice 4's (a matrix member of depth 2 is slice 3b's)
           k.strukt("U", {{"m", k.prog.array_of(k.prog.array_of(e1e2e_row(k), 3U), 2U)}});
       });
    // slice 4c is not built (depth 3 in every storage: the lowering's walkers read one inner level, ElemLeaf::count, and the trusted allocation is per row type): every form of it keeps its refusal
    rc("e1e2e_rej_depth4_local", "depth 4 or more is M8",
       [=](Kit& k)
       {
           // int c[2][2][2][2]: depth 4 is M8's
           Function* f {k.fn("f", k.vd)};
           Var* c {k.cell(f, "c",
                          k.prog.array_of(k.prog.array_of(k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U), 2U), 2U))};
           f->body = k.blkc({k.decl(c, nullptr)});
           k.done(f);
       });
    rc("e1e2e_rej_member_array_of_structures_with_a_matrix_member", "the member 'v' of the struct 'struct T'",
       [=](Kit& k)
       {
           // struct W { int m[2][2]; }; struct T { struct W v[2]; }: v[i].m[j][k] is depth 3 (slice 4c), so the structure W is refused as an element
           StructType* W {k.strukt("W", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)}})};
           k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}});
       });
    rc("e1e2e_rej_local_array_of_structures_with_a_matrix_member", "variable 'v' is an array of",
       [=](Kit& k)
       {
           // struct W { int m[2][2]; } v[2]; as a local
           StructType* W {k.strukt("W", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)}})};
           Function* f {k.fn("f", k.vd)};
           Var* v {k.cell(f, "v", k.prog.array_of(k.st(W), 2U))};
           f->body = k.blkc({k.decl(v, nullptr)});
           k.done(f);
       });
    rc("e1e2e_rej_matrix_of_structures_local", "an array of struct P[3]",
       [=](Kit& k)
       {
           // struct P { int x; } m[2][3]: a matrix of structures is slice 3's
           StructType* P {k.strukt("P", {{"x", k.i32}})};
           Function* f {k.fn("f", k.vd)};
           Var* m {k.cell(f, "m", k.prog.array_of(k.prog.array_of(k.st(P), 3U), 2U))};
           f->body = k.blkc({k.decl(m, nullptr)});
           k.done(f);
       });
    rc("e1e2e_rej_matrix_compound_literal", "an array compound literal of arrays",
       [=](Kit& k)
       {
           // (int[2][3]){...}: the hidden local of an array compound literal of arrays is not built
           Function* f {k.fn("f", k.vd)};
           Var* m {f->new_local("(int[2][3]){}", e1e2e_matrix(k), k.at())};
           f->mark_address_taken(m);
           m->compound_literal = true;
           f->body = k.blkc({k.decl(m, nullptr)});
           k.done(f);
       });
}

// ---- decision 0064 item 13 (Part E, slice 2a): a row PARAMETER in code ----
void define_part_e_row_parameter_code_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    // the address of a[i][j] of the parameter `a` (its own copy): `el(el(a, i), j)`, and as a read
    auto place = [=](Kit& k, Function* f, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.pv(f, 0), i, k.at()), j, k.at());
    };
    // void g(int (*b)[3]);  (declared, no contract)
    auto callee = [=](Kit& k)
    {
        Function* g {k.fn("g", k.vd, {{"b", rowptr(k)}})};
        k.trusted(g);
        return g;
    };
    lc("e1e2e_rowparam_unused", "void f(int (*a)[3]) {}: the parameter is accepted",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           f->body = k.blk({});
           k.done(f);
       });
    lc("e1e2e_rowparam_forwarded_whole",
       "void f(int (*a)[3]) { g(a); }: a row parameter given whole to a row parameter of the same row type; with nothing held through `a` the use fails its liveness row (2c adds the positive with a range atom)",
       [=](Kit& k)
       {
           Function* g {callee(k)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           const Expr* arg {k.pv(f, 0)};
           f->body = k.blk({k.es(k.call(g, {arg}))});
           k.done(f);
           // a pointer given to a callee is a use of it (0032 item 6): with nothing held through `a` its liveness row fails, at the argument (the range atoms of 2c give it)
           k.fail("lifetime", arg->loc);
       });
    lc("e1e2e_rowparam_matrix_passed",
       "int m[2][3]; g(m): the matrix's row 0 address is the whole argument; the call needs nothing of it",
       [=](Kit& k)
       {
           Function* g {callee(k)};
           Function* f {k.fn("f", k.vd)};
           Var* m {f->new_local("m", e1e2e_matrix(k), k.at())};
           f->mark_address_taken(m);
           m->zero_filled = true;
           f->body = k.blkc({k.decl(m, nullptr), k.es(k.call(g, {k.b.elem_addr(k.addr(m), N(k, "0"), k.at())}))});
           k.done(f);
       });
    lc("e1e2e_rowparam_write_without_atom_fail",
       "void f(int (*a)[3]) { a[1][2] = 3; } with no atom on `a`: the rows of a write at a constant cell: null-dereference, lifetime, both bounds, the cell's permission",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           const Stmt* w {k.store(place(k, f, N(k, "1"), N(k, "2")), N(k, "3"))};
           f->body = k.blk({w});
           k.done(f);
           k.fail("null-dereference", w->loc);
           k.fail("lifetime", w->loc);
           k.rows("out-of-bounds", w->loc, 2U);
           k.rows("permission", w->loc, 1U);
       });
    // int f(int (*a)[3], int i, int j) { return a[i][j]; }: nothing is held through `a`, so each row of the access fails: the pointer may be null, its block may have ended, both bounds are
    // unknown, the cell has no permission
    lc("e1e2e_rowparam_read_without_atom_fail",
       "int f(int (*a)[3], int i, int j) { return a[i][j]; } with no atom on `a`: null-dereference, lifetime, both bounds, the cell's permission",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"i", k.i32}, {"j", k.i32}})};
           const Expr* rd {k.deref(place(k, f, k.pv(f, 1), k.pv(f, 2)))};
           f->body = k.blk({k.ret(rd)});
           k.done(f);
           // the first row that fails is the pointer's null test, then its liveness; Silicon assumes each failed assertion, and the liveness assumption has no permission behind it, so the path
           // ends there: the later rows exist (the position map) and are not reported
           k.fail("null-dereference", rd->loc);
           k.fail("lifetime", rd->loc);
           k.rows("out-of-bounds", rd->loc, 2U);
           k.rows("permission", rd->loc, 1U);
           k.rows("uninitialized", rd->loc, 1U);
       });
    // int f(int (*a)[3]) { return 0; } with `&a[1][2]` formed as a value: one null row, one liveness row, the strict first bound, the second bound (one past the end allowed)
    lc("e1e2e_rowparam_address_formed_fail",
       "int *q = &a[1][2]; with no atom on `a`: the pointer may be null, its block ended, the row slot 1 outside the block, the row's length unknown",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           Var* q {k.local(f, "q", k.ptr(k.i32))};
           // the rows of `a` (null, lifetime, the first bound) are the row node's, at its own position; the second bound is the cell address's
           const Expr* row {k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at())};
           const Expr* ad {k.b.elem_addr(row, N(k, "2"), k.at())};
           f->body = k.blkc({k.decl(q, ad)});
           k.done(f);
           k.fail("null-dereference", row->loc);
           k.fail("lifetime", row->loc);
           k.rows("out-of-bounds", ad->loc, 2U);
           // record item 5: the rows at each index step in source order, the first index's bound before the row's own (the driver prints a case's errors sorted by message, which puts "beyond
           // one past the end of `a[1]`" before ", whose first index may be outside"; the Viper order is the source order)
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   const size_t first {vpr.find(
                       "assert !(((offset_of(v_a__0) + 1) < 0) || (len(block_of(v_a__0)) <= (offset_of(v_a__0) + 1)))")};
                   const size_t second {vpr.find(
                       "assert !((2 < 0) || (len(member(loc(block_of(v_a__0), offset_of(v_a__0) + 1), 0)) < 2))")};
                   if (first == std::string::npos || second == std::string::npos)
                   {
                       return "the two bound rows of &a[1][2] were not found";
                   }
                   return first < second ? "" : "the row's own bound row comes before the first index's";
               });
       });
    rc("e1e2e_rej_rowparam_assigned", "an assignment to the row parameter 'a'",
       [=](Kit& k)
       {
           // void f(int (*a)[3], int (*b)[3]) { a = b; }
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"b", rowptr(k)}})};
           f->body = k.blk({k.asg(f->param_var(0), k.pv(f, 1))});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_compared", "a pointer to int[3] (decision 0064 item 13",
       [=](Kit& k)
       {
           // int f(int (*a)[3]) { return a == a; }: the parameter is read only as the base of an element, an atom's base or a whole argument
           Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}})};
           f->body = k.blk({k.ret(k.bi(k.i32, k.eq(k.pv(f, 0), k.pv(f, 0))))});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_as_a_pointer_argument", "(convert it)",
       [=](Kit& k)
       {
           // void h(int *p); void f(int (*a)[3]) { h(a); }: a row pointer is no `int *`
           Function* h {k.fn("h", k.vd, {{"p", k.ptr(k.i32)}})};
           k.trusted(h);
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           f->body = k.blk({k.es(k.call(h, {k.pv(f, 0)}))});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_argument_of_another_row_type", "the parameter is",
       [=](Kit& k)
       {
           // void g4(int (*b)[4]); void f(int (*a)[3]) { g4(a); }
           Function* g {k.fn("g4", k.vd, {{"b", k.ptr(k.prog.array_of(k.i32, 4U))}})};
           k.trusted(g);
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           f->body = k.blk({k.es(k.call(g, {k.pv(f, 0)}))});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_row_address_argument", "a pointer to int[3] (decision 0064 item 13",
       [=](Kit& k)
       {
           // void g(int (*b)[3]); void f(int (*a)[3]) { g(&a[1]); }: only the whole parameter is an argument, never the address of one of its rows
           Function* g {callee(k)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           f->body = k.blk({k.es(k.call(g, {k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at())}))});
           k.done(f);
       });
    rc("e1e2e_rej_matrix_row_1_argument", "a pointer to int[3] (decision 0064 item 13",
       [=](Kit& k)
       {
           // void g(int (*b)[3]); int m[2][3]; g(&m[1]);  : only the row 0 address (the matrix's decay) is an argument
           Function* g {callee(k)};
           Function* f {k.fn("f", k.vd)};
           Var* m {f->new_local("m", e1e2e_matrix(k), k.at())};
           f->mark_address_taken(m);
           f->body = k.blkc({k.decl(m, nullptr), k.es(k.call(g, {k.b.elem_addr(k.addr(m), N(k, "1"), k.at())}))});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_marked", "carries `ref_to_uninit`, `must_init` or `not_invalidating`",
       [=](Kit& k)
       {
           // a row parameter with a marker
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           f->params[0].init = InitMarker::MustInit;
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_of_a_pure_function", "is a pointer to a row (decision 0064 item 14: Part F)",
       [=](Kit& k)
       {
           // bool p(int (*a)[3]) { return true; } as a `pure` function
           Function* f {k.fn("p", k.bl, {{"a", rowptr(k)}})};
           f->pure = true;
           f->pure_body = k.yes();
           k.done(f);
       });
    rc("e1e2e_rej_matrix_of_structures_parameter", "a pointer to struct P[2]",
       [=](Kit& k)
       {
           // void f(struct P (*a)[2]) {}: a row of structures (question 363: not built)
           StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"a", k.ptr(k.prog.array_of(k.st(P), 2U))}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_owns_cell_atom", "a pointer to int[3] (decision 0064 item 13",
       [=](Kit& k)
       {
           // void f(int (*a)[3]) requires(owns(*a)): a cell atom on a row parameter
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "a"), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_live_atom", "a pointer to int[3] (decision 0064 item 13",
       [=](Kit& k)
       {
           // void f(int (*a)[3]) requires(live(a)): a liveness atom on a row parameter
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           k.req(f, k.b.atom(AtomKind::Live, k.b.text(k.F(f, 0), "a"), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
}

// ---- decision 0064 items 4, 5 and 13 (Part E, slice 2b): `a[e][e2]` of a row parameter in a contract; trap 2: every index is part of the key ----
void define_part_e_row_parameter_contract_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    // the read `a[i][j]` of the formal `a` in a contract
    auto cread = [=](Kit& k, Function* f, const char* i, const char* j)
    {
        const Expr* row {k.b.elem_addr(k.F(f, 0), N(k, i), k.at())};
        return k.deref(k.b.elem_addr(row, N(k, j), k.at()));
    };
    // the distinct ghost names `hv_e<n>_pre` of the well-formedness method's formals
    auto ghosts = [](const std::string& vpr)
    {
        std::set<std::string> out;
        for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
        {
            size_t end {at + 4U};
            while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
            {
                ++end;
            }
            out.insert(vpr.substr(at, end - at));
        }
        return out.size();
    };
    // void f(int (*a)[3]) requires(a[0][1] == 3 && a[1][0] == 4 && a[1][1] == 5 && a[0][1] == 3)  { }: three distinct cells, one of them read twice
    lc("e1e2e_rowparam_contract_reads_keys",
       "requires a[0][1] == 3 && a[1][0] == 4 && a[1][1] == 5 && a[0][1] == 3 with no atom: the reads of a[0][1] and a[1][0] share neither row nor column in their keys, a[1][0] and a[1][1] share the row, a[0][1] twice is one key: three ghosts; the first unframed read is the permission error",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           const Expr* r0 {cread(k, f, "0", "1")};
           const Expr* r1 {cread(k, f, "1", "0")};
           const Expr* r2 {cread(k, f, "1", "1")};
           const Expr* r3 {cread(k, f, "0", "1")};
           k.req(f, k.land(k.land(k.eq(r0, N(k, "3")), k.eq(r1, N(k, "4"))),
                           k.land(k.eq(r2, N(k, "5")), k.eq(r3, N(k, "3")))));
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", r0->loc);
           k.rows("permission", r1->loc, 1U);
           k.rows("permission", r2->loc, 1U);
           k.rows("permission", r3->loc, 1U);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   const size_t n {ghosts(vpr)};
                   return n == 3U
                              ? ""
                              : "the ghosts of the four reads: expected 3 (one per cell), found " + std::to_string(n);
               });
       });
    lc("e1e2e_rowparam_contract_reads_ensures_old_decreases",
       "a[1][2] in a requires, an ensures, under old and in a decreases: each is a contract read, each its own row",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}})};
           const Expr* q {cread(k, f, "1", "2")};
           const Expr* e {cread(k, f, "1", "2")};
           const Expr* o {k.old(cread(k, f, "1", "2"))};
           const Expr* d {cread(k, f, "1", "2")};
           k.req(f, k.ge(q, N(k, "0")));
           k.ens(f, k.land(k.eq(k.R(f), e), k.eq(o, N(k, "0"))));
           k.dec(f, d);
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
           k.fail("permission", q->loc);
       });
    rc("e1e2e_rej_rowparam_row_address_in_a_contract",
       "the address of an array element in a contract is a read only as the operand",
       [=](Kit& k)
       {
           // void f(int (*a)[3]) requires(&a[1][0] != 0)... : an element address as a value in a contract
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           const Expr* row {k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at())};
           k.req(f, k.ne(k.b.elem_addr(row, N(k, "0"), k.at()), k.nul(k.ptr(k.i32))));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_rowparam_row_in_a_contract", "a pointer to int[3] (decision 0064 item 13",
       [=](Kit& k)
       {
           // requires(a == a) : the row parameter as a value in a contract
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           k.req(f, k.eq(k.F(f, 0), k.F(f, 0)));
           f->body = k.blk({});
           k.done(f);
       });
}

// ---- decision 0064 items 7, 8 and 13 (Part E, slice 2c): `owns_range(a, n)` and `reads_range(a, n)` over the rows of a row parameter; calls and returns ----
void define_part_e_row_range_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto RO = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    // the cell a[i][j] of the parameter's own copy (code)
    auto code_cell = [=](Kit& k, Function* f, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.pv(f, 0), i, k.at()), j, k.at());
    };
    // int f(int (*a)[3], int n) requires(owns_range(a, n) && n > <rows>) ensures(owns_range(a, n) && r == <want>) { a[<i>][<j>] = 7; return a[<i>][<j>]; }
    struct WR
    {
        const Stmt* store {nullptr};
        const Stmt* ret {nullptr};
    };
    const auto write_read = [=](Kit& k, const char* i, const char* j, const char* rows, const char* want,
                                bool reads) -> WR
    {
        Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"n", k.i32}})};
        k.req(f, k.land(reads ? RR(k, f, 0) : RO(k, f, 0), k.gt(fx(k, f, 1), N(k, rows))));
        k.ens(f, k.land(reads ? RR(k, f, 0) : RO(k, f, 0), k.eq(k.R(f), N(k, want))));
        WR out;
        out.store = k.store(code_cell(k, f, N(k, i), N(k, j)), N(k, "7"));
        out.ret = k.ret(k.deref(code_cell(k, f, N(k, i), N(k, j))));
        f->body = k.blkc({out.store, out.ret});
        k.done(f);
        return out;
    };
    lc("e1e2e_rowrange_write_read",
       "int f(int (*a)[3], int n) requires(owns_range(a, n) && n > 1) ensures(owns_range(a, n) && r == 7) { a[1][2] = 7; return a[1][2]; }: the cell is held, written, read, and returned with the range",
       [=](Kit& k)
       {
           write_read(k, "1", "2", "1", "7", false);
       });
    lc("e1e2e_rowrange_wrong_postcondition_fail", "the same ensuring r == 8: the postcondition at the return",
       [=](Kit& k)
       {
           const WR w {write_read(k, "1", "2", "1", "8", false)};
           k.fail("postcondition", w.ret->loc);
       });
    lc("e1e2e_rowrange_first_index_beyond_fail",
       "a[2][0] with n > 1: row 2 may lie outside the range, so neither bound is known (out-of-bounds twice: the row's slot, then the row's length, which only the range's facts give), and the cell is held by no atom (permission), all at the store",
       [=](Kit& k)
       {
           const WR w {write_read(k, "2", "0", "1", "7", false)};
           k.fail("out-of-bounds", w.store->loc, 2U);
           k.fail("permission", w.store->loc);
       });
    lc("e1e2e_rowrange_second_index_beyond_fail",
       "a[1][3] with n > 1: the second index against the row's length (3, by the atom's length facts): out-of-bounds once at the store; the rows after a failed bound are vacuous",
       [=](Kit& k)
       {
           const WR w {write_read(k, "1", "3", "1", "7", false)};
           k.fail_only("out-of-bounds", w.store->loc, 2U, 1U);
       });
    lc("e1e2e_rowrange_write_under_reads_fail", "reads_range(a, n): a[1][2] written: the write row (permission)",
       [=](Kit& k)
       {
           const WR w {write_read(k, "1", "2", "1", "7", true)};
           k.fail("permission", w.store->loc);
       });
    // void f(int (*a)[3], int n, int i, int j) requires(owns_range(a, n) && 0 <= i && i < n && 0 <= j && j <= <hi_j>) ensures(owns_range(a, n) && a[i][j] == 4)... as a[i][j] = 4;
    const auto symbolic = [=](Kit& k, const char* hi_j) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}, {"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.land(k.land(k.ge(fx(k, f, 2), N(k, "0")), k.lt(fx(k, f, 2), fx(k, f, 1))),
                                            k.land(k.ge(fx(k, f, 3), N(k, "0")), k.le(fx(k, f, 3), N(k, hi_j))))));
        k.ens(f, RO(k, f, 0));
        const Stmt* st {k.store(code_cell(k, f, k.pv(f, 2), k.pv(f, 3)), N(k, "4"))};
        f->body = k.blkc({st});
        k.done(f);
        return st;
    };
    lc("e1e2e_rowrange_symbolic_indexes", "0 <= i < n and 0 <= j <= 2: a[i][j] = 4 is held and written",
       [=](Kit& k)
       {
           symbolic(k, "2");
       });
    lc("e1e2e_rowrange_symbolic_second_index_fail",
       "0 <= i < n and 0 <= j <= 3: j == 3 is outside the row: out-of-bounds once at the store",
       [=](Kit& k)
       {
           const Stmt* st {symbolic(k, "3")};
           k.fail_only("out-of-bounds", st->loc, 2U, 1U);
       });
    // int f(int (*a)[3], int n) requires(owns_range(a, n) && <n_cond> && a[1][2] == 5) ensures(owns_range(a, n) && r == 5 && a[1][2] == 5) { return a[1][2]; }
    const auto contract = [=](Kit& k, const char* rows, bool wrong) -> std::pair<const Expr*, const Stmt*>
    {
        Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"n", k.i32}})};
        const Expr* rq {k.deref(k.b.elem_addr(k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at()), N(k, "2"), k.at()))};
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, rows))), k.eq(rq, N(k, "5"))));
        const Expr* re {k.deref(k.b.elem_addr(k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at()), N(k, "2"), k.at()))};
        k.ens(f, k.land(k.land(RO(k, f, 0), k.eq(k.R(f), N(k, wrong ? "6" : "5"))), k.eq(re, N(k, "5"))));
        const Stmt* ret {k.ret(k.deref(code_cell(k, f, N(k, "1"), N(k, "2"))))};
        f->body = k.blk({ret});
        k.done(f);
        return {rq, ret};
    };
    lc("e1e2e_rowrange_contract_read_framed",
       "requires owns_range(a, n) && n > 1 && a[1][2] == 5: the read is framed (row 1 is in the range, column 2 is in the row), the body returns it, the ensures reads it again",
       [=](Kit& k)
       {
           contract(k, "1", false);
       });
    lc("e1e2e_rowrange_contract_read_unframed_fail",
       "the same with n > 0 only: row 1 may lie outside the range, so the read in the requires is unowned (permission, in the well-formedness of the contract)",
       [=](Kit& k)
       {
           k.fail("permission", contract(k, "0", false).first->loc);
       });
    lc("e1e2e_rowrange_contract_read_wrong_result_fail", "the same ensuring r == 6: the postcondition",
       [=](Kit& k)
       {
           k.fail("postcondition", contract(k, "1", true).second->loc);
       });
    // the second bound of a contract read: requires(owns_range(a, n) && n > 1 && a[1][<col>] == 5): column 3 is outside the row of 3 cells, so the framing (a cell of a row at a slot in the range) does not hold
    lc("e1e2e_rowrange_contract_read_second_index_fail",
       "requires owns_range(a, n) && n > 1 && a[1][3] == 5: the read is of no cell of the row (the framing needs 0 <= j < 3): unowned (permission)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
           const Expr* rq {k.deref(k.b.elem_addr(k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at()), N(k, "3"), k.at()))};
           k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(rq, N(k, "5"))));
           k.ens(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", rq->loc);
       });
    // int f(int (*a)[3], int n) requires(owns_range(a, n) && n > 1 && a[1][2] == 5) ensures(owns_range(a, n) && old(a[1][2]) == <old_want> && r == 5) decreases(a[1][2]) { return a[1][2]; }
    const auto old_dec = [=](Kit& k, const char* old_want) -> const Stmt*
    {
        Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"n", k.i32}})};
        auto cr = [&]()
        {
            return k.deref(k.b.elem_addr(k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at()), N(k, "2"), k.at()));
        };
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(cr(), N(k, "5"))));
        k.ens(f, k.land(k.land(RO(k, f, 0), k.eq(k.old(cr()), N(k, old_want))), k.eq(k.R(f), N(k, "5"))));
        k.dec(f, cr());
        const Stmt* ret {k.ret(k.deref(code_cell(k, f, N(k, "1"), N(k, "2"))))};
        f->body = k.blk({ret});
        k.done(f);
        return ret;
    };
    lc("e1e2e_rowrange_contract_read_old_and_decreases",
       "a[1][2] == 5 in the requires: old(a[1][2]) == 5 in the ensures and a[1][2] as the decreases measure are framed by the range and verify",
       [=](Kit& k)
       {
           old_dec(k, "5");
       });
    lc("e1e2e_rowrange_contract_read_old_wrong_fail",
       "the same claiming old(a[1][2]) == 6: the postcondition fails at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", old_dec(k, "6")->loc);
       });
    // ---- calls: void wr(int (*b)[3], int m) requires(owns_range(b, m)[ && m > 1]) ensures(owns_range(b, m)); (declared) ----
    auto callee = [=](Kit& k, bool reads, bool own_it) -> Function*
    {
        Function* g {k.fn(reads ? "rd" : "wr", k.vd, {{"b", rowptr(k)}, {"m", k.i32}})};
        k.req(g, own_it ? RO(k, g, 0) : RR(k, g, 0));
        k.ens(g, own_it ? RO(k, g, 0) : RR(k, g, 0));
        k.trusted(g);
        return g;
    };
    // void f(int (*a)[3], int n) requires(owns_range(a, n) && n > 1) ensures(owns_range(a, n)) { wr(a, <count>); }  (the caller's range, forwarded or cut)
    const auto forward = [=](Kit& k, bool reads, const char* count_text) -> const Expr*
    {
        Function* g {callee(k, reads, !reads)};
        Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
        k.req(f, k.land(reads ? RR(k, f, 0) : RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, reads ? RR(k, f, 0) : RO(k, f, 0));
        const Expr* cnt {count_text == nullptr ? k.pv(f, 1) : N(k, count_text)};
        const Expr* callx {k.call(g, {k.pv(f, 0), cnt})};
        f->body = k.blk({k.es(callx)});
        k.done(f);
        return callx;
    };
    lc("e1e2e_rowrange_call_forward_owns",
       "a row parameter and its count given whole to a callee that owns the range and gives it back",
       [=](Kit& k)
       {
           forward(k, false, nullptr);
       });
    lc("e1e2e_rowrange_call_forward_reads", "the same for reads_range: the callee borrows the rows and returns them",
       [=](Kit& k)
       {
           forward(k, true, nullptr);
       });
    // void f(int (*a)[3], int n) requires(owns_range(a, n) && n > 1) ensures(owns_range(a, n)) { wr(a, n + 1); }  (the caller holds n rows and passes n + 1)
    lc("e1e2e_rowrange_call_forward_past_fail",
       "the caller's n rows forwarded as n + 1: the callee's shape row, the length fact of the row n and the permission of its cells fail at the call (the pin of tests/memory/negative/array_rows_forward_past.c)",
       [=](Kit& k)
       {
           Function* g {callee(k, false, true)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.land(k.gt(fx(k, f, 1), N(k, "1")), k.lt(fx(k, f, 1), N(k, "100")))));
           k.ens(f, RO(k, f, 0));
           const Expr* callx {k.call(g, {k.pv(f, 0), k.add(k.pv(f, 1), N(k, "1"))})};
           f->body = k.blk({k.es(callx)});
           k.done(f);
           // nine permission rows at the call, three of which fail (the first path-ending marked row ends it): the shape row (`offset + n + 1 <= len`), the length fact of the rows
           // `[offset, offset + n + 1)` (both: "calls `wr`, which needs to own the range ..., without owning it") and the value permission of the cells of every row (the same text, with
           // " (every cell of every row)"); the driver prints one error for the first two (one position, one message) and one for the third: three rows, two errors
           k.fail_only("permission", callx->loc, 9U, 0U);
           k.fail_only("permission", callx->loc, 9U, 4U);
           k.fail_only("permission", callx->loc, 9U, 5U);
       });
    // (a parameter's range cut to 3 rows where the caller holds only n > 1 fails three rows, the shape row, the length fact of the third row and its cells' permission; the matrix case below, whose block length is known, fails the shape row alone)
    lc("e1e2e_rowrange_call_reads_given_owns_needed_fail",
       "a caller that only reads the rows gives them to a callee that owns them: the marked rows at the call (permission)",
       [=](Kit& k)
       {
           Function* g {callee(k, false, true)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RR(k, f, 0));
           const Expr* callx {k.call(g, {k.pv(f, 0), k.pv(f, 1)})};
           f->body = k.blk({k.es(callx)});
           k.done(f);
           // a failed marked row ends its path (S43 G2): Silicon reports the first, the value permission of the row cells (the sixth row of the call)
           k.fail_only("permission", callx->loc, 9U, 5U);
       });
    // int f(void) { int m[<rows>][3] = {0}; rd(m, <count>); check(m[1][2] == <want>); return 0; }  with rd reading the range: the values it borrowed are kept
    const auto matrix_call = [=](Kit& k, const char* count, const char* want) -> std::pair<const Expr*, const Stmt*>
    {
        Function* g {callee(k, true, false)};
        Function* f {k.fn("f", k.i32)};
        Var* m {f->new_local("m", e1e2e_matrix(k), k.at())};
        f->mark_address_taken(m);
        m->zero_filled = true;
        const Expr* callx {k.call(g, {k.b.elem_addr(k.addr(m), N(k, "0"), k.at()), N(k, count)})};
        const Stmt* chk {k.chk(
            k.eq(k.deref(k.b.elem_addr(k.b.elem_addr(k.addr(m), N(k, "1"), k.at()), N(k, "2"), k.at())), N(k, want)))};
        f->body = k.blkc({k.decl(m, nullptr), k.es(callx), chk, k.ret(N(k, "0"))});
        k.done(f);
        return {callx, chk};
    };
    lc("e1e2e_rowrange_matrix_to_a_reads_callee",
       "int m[2][3] = {0}; rd(m, 2); check(m[1][2] == 0): the matrix's row 0 address is the whole argument, both rows are borrowed and returned, the value is kept",
       [=](Kit& k)
       {
           matrix_call(k, "2", "0");
       });
    lc("e1e2e_rowrange_matrix_to_a_reads_callee_wrong_value_fail", "the same checking m[1][2] == 1: the check fails",
       [=](Kit& k)
       {
           k.fail("check", matrix_call(k, "2", "1").second->loc);
       });
    lc("e1e2e_rowrange_matrix_too_many_rows_fail",
       "the same giving the callee 3 rows of a matrix of 2: the shape row (permission) at the call",
       [=](Kit& k)
       {
           k.fail_only("permission", matrix_call(k, "3", "0").first->loc, 10U, 0U);
       });
}

// ---- decision 0064 item 7 (Part E, slice 2d): the pairing rows for rows, Form P, per pair of leaf paths by slots; the sequencing rows of a call that holds a range over rows ----
void define_part_e_row_pairing_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto matrix_local = [](Kit& k, Function* f, const char* name, bool zero, unsigned long long rows = 3U)
    {
        Var* m {f->new_local(name, k.prog.array_of(e1e2e_row(k), rows), k.at())};
        f->mark_address_taken(m);
        m->zero_filled = zero;
        return m;
    };
    // the address of row 0 of a local matrix (the matrix passed to a row parameter), the decay of its row `r` (`m[r]`) and the address of its cell (`&m[r][c]`)
    auto row0 = [](Kit& k, Var* m)
    {
        return k.b.elem_addr(k.addr(m), k.num(k.i32, "0"), k.at());
    };
    auto decay = [](Kit& k, Var* m, const char* r)
    {
        return k.b.elem_addr(k.b.elem_addr(k.addr(m), k.num(k.i32, r), k.at()), k.num(k.i32, "0"), k.at());
    };
    auto cell_addr = [](Kit& k, Var* m, const char* r, const char* c)
    {
        return k.b.elem_addr(k.b.elem_addr(k.addr(m), k.num(k.i32, r), k.at()), k.num(k.i32, c), k.at());
    };
    // void c2(int (*b)[3], int n, int *q) requires(<range>(b, n) && <cell>(*q) && n > 0) ensures(<range>(b, n) && <cell>(*q)) {}
    auto mk_c2 = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind) -> Function*
    {
        Function* c {k.fn("c2", k.vd, {{"b", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
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
        k.trusted(c);
        return c;
    };
    // void f(void) { int m[3][3] = {0}; int c = 0; c2(m, <n>, <q>); }, q the local's address (r < 0) or &m[r][col]
    const auto pair_cell = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind, const char* n, int r,
                               const char* col) -> SrcLoc
    {
        Function* c2 {mk_c2(k, range_kind, cell_kind)};
        Function* f {k.fn("f", k.vd)};
        Var* m {matrix_local(k, f, "m", true)};
        Var* c {k.cell(f, "c", k.i32)};
        const Expr* q {r < 0 ? k.addr(c) : cell_addr(k, m, std::to_string(r).c_str(), col)};
        const Expr* call {k.call(c2, {row0(k, m), N(k, n), q})};
        f->body = k.blkc({k.decl(m, nullptr), k.decl(c, N(k, "0")), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2e_rowpair_cell_outside_range",
       "c2(m, 2, &c) with c a separate local, owns_range(b, n) && owns(*q): the cell is no cell of the rows: the row passes",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "2", -1, "0");
       });
    lc("e1e2e_rowpair_cell_in_a_later_row",
       "c2(m, 2, &m[2][1]): the cell is in row 2, outside the range [0, 2): the row (the row index recovered by one parent_of, the member step 0 between the index steps) passes",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "2", 2, "1");
       });
    lc("e1e2e_rowpair_cell_alias_fail",
       "c2(m, 3, &m[1][2]): the cell is a cell of row 1, which the range holds: the row fails (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "3", 1, "2"), 1U, 0U,
                       "which may be one object");
       });
    lc("e1e2e_rowpair_cell_alias_first_column_fail",
       "c2(m, 3, &m[2][0]): the same through the first cell of the last row",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "3", 2, "0"), 1U, 0U,
                       "which may be one object");
       });
    lc("e1e2e_rowpair_reads_range_owns_cell_alias_fail",
       "c2 reads the rows and owns the cell: c2(m, 3, &m[1][2]): an owner among the two sites, the row fails once",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::ReadsRange, AtomKind::Owns, "3", 1, "2"), 1U, 0U,
                       "which may be one object");
       });
    // void s2(int (*b)[3], int n, int *q, int c) requires(owns_range(b, n) && owns_range(q, c) && n > 0 && c > 0) ensures(owns_range(b, n) && owns_range(q, c)) {}
    // void f(void) { int m[3][3] = {0}; s2(m, <n>, m[<r>], 3); }: a range over rows against a scalar range over one row
    const auto pair_range = [=](Kit& k, const char* n, const char* r) -> SrcLoc
    {
        Function* s2 {k.fn("s2", k.vd, {{"b", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}, {"c", k.i32}})};
        auto ro = [&](int base, int count, int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, s2, base), fx(k, s2, count), amount, k.at());
        };
        k.req(s2, k.land(k.land(ro(0, 1, 0), ro(2, 3, 1)),
                         k.land(k.gt(fx(k, s2, 1), N(k, "0")), k.gt(fx(k, s2, 3), N(k, "0")))));
        k.ens(s2, k.land(ro(0, 1, 0), ro(2, 3, 1)));
        k.trusted(s2);
        Function* f {k.fn("f", k.vd)};
        Var* m {matrix_local(k, f, "m", true)};
        const Expr* call {k.call(s2, {row0(k, m), N(k, n), decay(k, m, r), N(k, "3")})};
        f->body = k.blkc({k.decl(m, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2e_rowpair_range_row_outside",
       "s2(m, 2, m[2], 3): the scalar range is row 2's cells, outside the rows [0, 2): the row (the range's first cell against the rows' leaf) passes",
       [=](Kit& k)
       {
           pair_range(k, "2", "2");
       });
    lc("e1e2e_rowpair_range_row_inside_fail",
       "s2(m, 3, m[1], 3): the scalar range is row 1's cells, which the rows [0, 3) hold: the row fails (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_range(k, "3", "1"), 1U, 0U, "which may be one object");
       });
    // void g2(int (*b)[3], int n, int (*c)[3], int k) requires(owns_range(b, n) && owns_range(c, k) && n > 0 && k > 0) ensures(...) {}
    // void f(void) { int m[2][3] = {0}; int p[2][3] = {0}; g2(m, 2, <second>, <k>); }: two ranges over rows
    const auto pair_two = [=](Kit& k, bool same, const char* count) -> SrcLoc
    {
        Function* g2 {k.fn("g2", k.vd, {{"b", rowptr(k)}, {"n", k.i32}, {"c", rowptr(k)}, {"k", k.i32}})};
        auto ro = [&](int base, int cnt, int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, g2, base), fx(k, g2, cnt), amount, k.at());
        };
        k.req(g2, k.land(k.land(ro(0, 1, 0), ro(2, 3, 1)),
                         k.land(k.gt(fx(k, g2, 1), N(k, "0")), k.gt(fx(k, g2, 3), N(k, "0")))));
        k.ens(g2, k.land(ro(0, 1, 0), ro(2, 3, 1)));
        k.trusted(g2);
        Function* f {k.fn("f", k.vd)};
        Var* m {matrix_local(k, f, "m", true, 2U)};
        Var* p {matrix_local(k, f, "p", true, 2U)};
        const Expr* call {k.call(g2, {row0(k, m), N(k, "2"), row0(k, same ? m : p), N(k, count)})};
        f->body = k.blkc({k.decl(m, nullptr), k.decl(p, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2e_rowpair_two_ranges_disjoint", "g2(m, 2, p, 2) over two matrices: the slot row passes",
       [=](Kit& k)
       {
           pair_two(k, false, "2");
       });
    lc("e1e2e_rowpair_two_ranges_overlap_fail",
       "g2(m, 2, m, 1): the two ranges share row 0: the slot row fails (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_two(k, true, "1"), 1U, 0U, "which may be one object");
       });
    // int w(int (*b)[3], int n) requires(owns_range(b, n) && n > 0) ensures(owns_range(b, n)); int f(void) { int m[3][3] = {0}; int p[3][3] = {0}; w(m, 2) == <other>; }
    auto mk_w = [=](Kit& k) -> Function*
    {
        Function* w {k.fn("w", k.i32, {{"b", rowptr(k)}, {"n", k.i32}})};
        auto ro = [&](int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, w, 0), fx(k, w, 1), amount, k.at());
        };
        k.req(w, k.land(ro(0), k.gt(fx(k, w, 1), N(k, "0"))));
        k.ens(w, ro(0));
        k.trusted(w);
        return w;
    };
    // w(m, 2) == m[<r>][0]: the call holds the range over rows 0 and 1; the read is the other operand of one full expression
    const auto seq_read = [=](Kit& k, const char* r) -> const Expr*
    {
        Function* w {mk_w(k)};
        Function* f {k.fn("f", k.vd)};
        Var* m {matrix_local(k, f, "m", true)};
        const Expr* rd {k.deref(cell_addr(k, m, r, "0"))};
        const Expr* callx {k.call(w, {row0(k, m), N(k, "2")})};
        const Expr* both {k.eq(callx, rd)};
        f->body = k.blkc({k.decl(m, nullptr), k.es(both)});
        k.done(f);
        return rd;
    };
    lc("e1e2e_rowseq_call_and_read_disjoint",
       "w(m, 2) == m[2][0]: the read is of row 2, outside the call's rows: the unsequenced row passes",
       [=](Kit& k)
       {
           seq_read(k, "2");
       });
    lc("e1e2e_rowseq_call_and_read_overlap_fail",
       "w(m, 2) == m[1][0]: the read is of row 1, which the call holds: the unsequenced row fails",
       [=](Kit& k)
       {
           const Expr* rd {seq_read(k, "1")};
           k.fail("unsequenced", rd->loc);
       });
    // w(m, 2) == w(<other>, <count>): two calls in one full expression
    const auto seq_calls = [=](Kit& k, bool same, const char* count) -> const Expr*
    {
        Function* w {mk_w(k)};
        Function* f {k.fn("f", k.vd)};
        Var* m {matrix_local(k, f, "m", true)};
        Var* p {matrix_local(k, f, "p", true)};
        const Expr* c1 {k.call(w, {row0(k, m), N(k, "2")})};
        const Expr* c2 {k.call(w, {row0(k, same ? m : p), N(k, count)})};
        f->body = k.blkc({k.decl(m, nullptr), k.decl(p, nullptr), k.es(k.eq(c1, c2))});
        k.done(f);
        return c2;
    };
    lc("e1e2e_rowseq_two_calls_disjoint",
       "w(m, 2) == w(p, 2): two matrices: the unsequenced row between the two calls passes",
       [=](Kit& k)
       {
           seq_calls(k, false, "2");
       });
    lc("e1e2e_rowseq_two_calls_overlap_fail",
       "w(m, 2) == w(m, 1): the calls hold row 0 both: the unsequenced row fails",
       [=](Kit& k)
       {
           k.fail("unsequenced", seq_calls(k, true, "1")->loc);
       });
}

// ---- slice 2c/2d, functions WITH BODIES: a row range beside a second atom, forwarded, returned and read through unatomed pointers (the audit of the walkers the plan names: entry_checks, atom_returns, range_returns, f__wf) ----
void define_part_e_row_body_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // void c2(int (*b)[3], int n, int *q) requires(<range>(b, n) && <cell>(*q) && n > 0) ensures(<range>(b, n) && <cell>(*q)) <body>
    auto mk_c2 = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind, bool trusted) -> Function*
    {
        Function* c {k.fn("c2", k.vd, {{"b", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
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
        if (trusted)
        {
            k.trusted(c);
        }
        else
        {
            c->body = k.blkc({});
            k.done(c);
        }
        return c;
    };
    // void h2(int (*b)[3], int n, int (*c)[3], int m) requires(<k1>(b, n) && <k2>(c, m) && n > 0 && m > 0) ensures(the same) <body>
    auto mk_h2 = [=](Kit& k, AtomKind k1, AtomKind k2, bool trusted) -> Function*
    {
        Function* h {k.fn("h2", k.vd, {{"b", rowptr(k)}, {"n", k.i32}, {"c", rowptr(k)}, {"m", k.i32}})};
        auto ra = [&](int pi, int ni, AtomKind kind, int amount)
        {
            return k.b.range_atom(kind, fx(k, h, pi), fx(k, h, ni), amount, k.at());
        };
        k.req(h, k.land(k.land(ra(0, 1, k1, 0), ra(2, 3, k2, 1)),
                        k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
        k.ens(h, k.land(ra(0, 1, k1, 0), ra(2, 3, k2, 1)));
        if (trusted)
        {
            k.trusted(h);
        }
        else
        {
            h->body = k.blkc({});
            k.done(h);
        }
        return h;
    };
    lc("e1e2e_rowbody_returns_owns_range_and_cell",
       "c2 = owns_range(b, n) over rows && owns(*q) with an empty body: the return rows give both back (the cell sum against the range's leaf, the range's rows)",
       [=](Kit& k)
       {
           mk_c2(k, AtomKind::OwnsRange, AtomKind::Owns, false);
       });
    lc("e1e2e_rowbody_returns_reads_range_and_cell",
       "c2 = reads_range(b, n) && reads(*q) with an empty body: the return rows add the cell's amount at the rows' cells and the range's at the cell (the j-binder amount of range_returns)",
       [=](Kit& k)
       {
           mk_c2(k, AtomKind::ReadsRange, AtomKind::Reads, false);
       });
    lc("e1e2e_rowbody_returns_two_reads_ranges",
       "h2 = reads_range(b, n) && reads_range(c, m) over rows, empty body: the two ranges may meet, so each marked row adds the other's amount where it covers the slot (the sum form)",
       [=](Kit& k)
       {
           mk_h2(k, AtomKind::ReadsRange, AtomKind::ReadsRange, false);
       });
    lc("e1e2e_rowbody_returns_owns_and_reads_ranges",
       "h2 = owns_range(b, n) && reads_range(c, m) over rows, empty body",
       [=](Kit& k)
       {
           mk_h2(k, AtomKind::OwnsRange, AtomKind::ReadsRange, false);
       });
    // void f(int (*a)[3], int n, int *q) requires(owns_range(a, n) && owns(*q) && n > 0) ensures(...) { c2(a, n, q); }: the call's rows are decided from f's own permissions
    lc("e1e2e_rowbody_forward_cell",
       "f forwards its owns_range(a, n) and owns(*q) to c2: the cell row of the leaf is proved from f's two full permissions (no alias is possible)",
       [=](Kit& k)
       {
           Function* c2 {mk_c2(k, AtomKind::OwnsRange, AtomKind::Owns, true)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
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
    lc("e1e2e_rowbody_forward_two_ranges",
       "f forwards its two owned ranges over rows to h2: the slot row is proved from f's permissions (no leaf receiver of a first slot is in view: the case the block row could not decide)",
       [=](Kit& k)
       {
           Function* h {mk_h2(k, AtomKind::OwnsRange, AtomKind::OwnsRange, true)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}, {"b", rowptr(k)}, {"m", k.i32}})};
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
    lc("e1e2e_rowbody_forward_one_range_twice_fail",
       "f(a, n) holds ONE range and forwards it as both ranges of h2(a, n, a, n): the slot row fails (permission), the same object twice",
       [=](Kit& k)
       {
           Function* h {mk_h2(k, AtomKind::OwnsRange, AtomKind::OwnsRange, true)};
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
           auto ra = [&](int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
           };
           k.req(f, k.land(ra(0), k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, ra(0));
           const Expr* callx {k.call(h, {k.pv(f, 0), k.pv(f, 1), k.pv(f, 0), k.pv(f, 1)})};
           f->body = k.blkc({k.es(callx)});
           k.done(f);
           k.fail_only("permission", callx->loc, 1U, 0U, "which may be one object");
       });
    // ---- a `cerastes_forall` beside a range over rows (record 0061 item 11, record 0064 item 7): the row range frames no read under a binder ----
    // bool zero_at(int k, const int *p, int m) { return !(0 <= k && k < m) || p[k] == 0; }, forall zero_at(p, m) in the contract
    auto mkq = [=](Kit& k, int id, const std::vector<const Expr*>& args)
    {
        const Expr* i {k.b.bound(id, k.i32.int_type, "i", k.at())};
        std::vector<const Expr*> a;
        const std::vector<Type> ptypes {k.ptr(k.i32), k.i32};
        for (size_t j {0U}; j < args.size(); ++j)
        {
            a.push_back(k.b.forall_arg(id, static_cast<int>(j), ptypes[j],
                                       args[j]->c_text.empty() ? "x" : args[j]->c_text, k.at()));
        }
        const Expr* elem {k.deref(k.b.elem_addr(a[0], i, k.at()))};
        const Expr* b {k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(i, a[1]))), k.eq(elem, N(k, "0")))};
        return k.b.forall(id, k.i32.int_type, b, args, "forall(zero_at)", "zero_at", k.at());
    };
    // void f(int (*a)[3], int n, int *p, int m) requires(owns_range(a, n) [&& owns_range(p, m)] && n > 0 && m > 0 && forall zero_at(p, m)) ensures(owns_range(a, n) [&& owns_range(p, m)]) {}
    const auto forall_case = [=](Kit& k, bool scalar_range) -> const Expr*
    {
        Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}, {"p", k.ptr(k.i32)}, {"m", k.i32}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
        };
        auto rp = [&](int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 2), fx(k, f, 3), amount, k.at());
        };
        const Expr* q {mkq(k, 0, {fx(k, f, 2), fx(k, f, 3)})};
        const Expr* sites {scalar_range ? k.land(ra(0), rp(1)) : ra(0)};
        k.req(f, k.land(k.land(sites, k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))), q));
        k.ens(f, scalar_range ? k.land(ra(0), rp(1)) : ra(0));
        f->body = k.blk({});
        k.done(f);
        return q;
    };
    lc("e1e2e_rowbody_forall_beside_a_row_range_alone_fail",
       "forall zero_at(p, m) reads p[j] under its binder and the only range is over rows: the rows frame no read under a binder (permission at the quantifier, never an internal error)",
       [=](Kit& k)
       {
           k.fail("permission", forall_case(k, false)->loc);
       });
    lc("e1e2e_rowbody_forall_beside_a_row_range_framed_by_scalar_range",
       "the same beside owns_range(p, m): the read under the binder is framed by the scalar range alone",
       [=](Kit& k)
       {
           forall_case(k, true);
       });
    // a row range beside a pointer with no atom: f(int (*a)[3], int n, int *q) requires(owns_range(a, n) && n > 0) ensures(owns_range(a, n)) {}
    lc("e1e2e_rowbody_range_beside_unatomed_pointer",
       "a row range and an int * parameter with no atom of its own: the entry's case splits and the return rows handle the unatomed pointer (it reads the cell field of the rows)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
           k.req(f, k.land(k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                           k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    // int f(int (*a)[3], int n, int *q) requires(owns_range(a, n) && n > 0) { return *q; }: *q may be a cell of the rows or none (no atom on q): the read is unowned (permission)
    lc("e1e2e_rowbody_unatomed_pointer_read_fail",
       "the same reading *q: q has no atom, the rows are no promise about it (permission at the read)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
           k.req(f, k.land(k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                           k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
           const Expr* rd {k.deref(k.pv(f, 2))};
           f->body = k.blk({k.ret(rd)});
           k.done(f);
           k.fail("null-dereference", rd->loc);
           k.fail("permission", rd->loc);
       });
    // void f(int (*a)[3], int *q) requires(owns(*q)) ensures(owns(*q)) {}: a row parameter with no atom beside a cell atom
    lc("e1e2e_rowbody_row_parameter_without_atom_beside_a_cell",
       "a row parameter with no atom and an owned int cell: the entry's case splits on the unatomed row pointer",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"q", k.ptr(k.i32)}})};
           k.req(f, k.b.atom(AtomKind::Owns, fx(k, f, 1), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fx(k, f, 1), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
}

// ---- decision 0064 items 10 and 13 (Part E, slice 2e): loops over a local matrix and over a row parameter's range ----
void define_part_e_loop_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
    };
    // ---- a local matrix ----
    auto mat = [](Kit& k, Function* f, bool zero)
    {
        Var* m {f->new_local("m", e1e2e_matrix(k), k.at())};
        f->mark_address_taken(m);
        m->zero_filled = zero;
        return m;
    };
    auto mplace = [](Kit& k, Var* m, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.addr(m), i, k.at()), j, k.at());
    };
    auto mread = [=](Kit& k, Var* m, const Expr* i, const Expr* j)
    {
        return k.deref(mplace(k, m, i, j));
    };
    // int f(void) { int m[2][3] = {0}; int i = 0; int j = 0;
    //   while (i < 2) inv 0 <= i <= 2 [&& m[1][2] == 0] decreases 2 - i { j = 0; while (j < 3) inv 0 <= j <= 3 [&& (j >= 3 || m[i][j] == 0)] [&& m[1][2] == 0] decreases 3 - j { [m[i][j] = 1;] j++; } i++; }
    //   check(m[0][1] == <want>); return 0; }
    // `mode`: 0 the reads (the invariants keep the zeros), 1 want 1, 2 check(false) after the loops, 3 check(false) at the end of the inner body
    const auto lnest = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib;
        if (c3 != nullptr)
        {
            ib.push_back(c3);
        }
        ib.push_back(inc(k, j));
        const Stmt* inner {k.whl(k.lt(k.v(j), N(k, "3")),
                                 {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3")))),
                                  k.cl(k.lor(k.ge(k.v(j), N(k, "3")), k.eq(mread(k, m, k.v(i), k.v(j)), N(k, "0")))),
                                  k.cl(k.eq(mread(k, m, N(k, "1"), N(k, "2")), N(k, "0")))},
                                 {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc(ib))};
        const Stmt* outer {k.whl(k.lt(k.v(i), N(k, "2")),
                                 {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2"))))},
                                 {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc({k.asg(j, N(k, "0")), inner, inc(k, i)}))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        const Stmt* c1 {k.chk(k.eq(mread(k, m, N(k, "0"), N(k, "1")), N(k, mode == 1 ? "1" : "0")))};
        std::vector<const Stmt*> body {k.decl(m, nullptr), k.decl(i, N(k, "0")), k.decl(j, N(k, "0")), outer, c1};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        body.push_back(k.ret(N(k, "0")));
        f->body = k.blkc(std::move(body));
        k.done(f);
        return mode == 1 ? c1 : mode == 2 ? c2 : mode == 3 ? c3 : nullptr;
    };
    lc("e1e2e_loop_matrix_nested_reads",
       "two nested loops over int m[2][3] = {0} that only read: one array place for the matrix (its cells over two bound variables), the invariants read m[1][2] and m[i][j] (both bounds, the cell's flag), the zeros are kept",
       [=](Kit& k)
       {
           lnest(k, 0);
       });
    lc("e1e2e_loop_matrix_nested_wrong_fail", "the same loops, then check(m[0][1] == 1): fails at the check",
       [=](Kit& k)
       {
           k.fail("check", lnest(k, 1)->loc);
       });
    lc("e1e2e_loop_matrix_nested_vacuity", "the same loops with check(false) after them: it fails",
       [=](Kit& k)
       {
           k.fail("check", lnest(k, 2)->loc);
       });
    lc("e1e2e_loop_matrix_nested_body_vacuity",
       "the same loops with check(false) at the end of the inner body: it fails",
       [=](Kit& k)
       {
           k.fail("check", lnest(k, 3)->loc);
       });
    // int f(void) { int m[2][3] = {0}; int i = 0; while (i < 3) inv 0 <= i <= 3 && m[1][2] == 0 decreases 3 - i { m[1][i] = 1; i++; } return 0; }
    const auto lwrite = [=](Kit& k) -> const Clause
    {
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        Var* i {k.local(f, "i", k.i32)};
        const Clause keep {k.cl(k.eq(mread(k, m, N(k, "1"), N(k, "2")), N(k, "0")))};
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")),
                                {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                k.blkc({k.store(mplace(k, m, N(k, "1"), k.v(i)), N(k, "1")), inc(k, i)}))};
        f->body = k.blkc({k.decl(m, nullptr), k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return keep;
    };
    lc("e1e2e_loop_matrix_write_loses_values_fail",
       "the loop writes m[1][i]: a store by name reaches the matrix's place, so m[1][2] == 0 is not kept (the invariant fails at the back edge, once)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", lwrite(k).loc);
       });
    // int f(void) { int m[2][3] = {0}; int j = 0; int s = 0; while (j <= <hi>) inv 0 <= j <= 4 decreases 4 - j { s = m[0][j]; j++; } return 0; }
    lc("e1e2e_loop_matrix_second_index_high_fail",
       "the loop reads m[0][j] up to j == 3: the second index against the row's own length fails at the read (out-of-bounds, the second of the two rows)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, true)};
           Var* j {k.local(f, "j", k.i32)};
           Var* s {k.local(f, "s", k.i32)};
           const Expr* rdx {mread(k, m, N(k, "0"), k.v(j))};
           const Stmt* rd {k.asg(s, rdx)};
           const Stmt* loop {k.whl(k.le(k.v(j), N(k, "3")),
                                   {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "4"))))},
                                   {k.cl(k.sub(N(k, "4"), k.v(j)))}, k.blkc({rd, inc(k, j)}))};
           f->body = k.blkc({k.decl(m, nullptr), k.decl(j, N(k, "0")), k.decl(s, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
           k.fail_only("out-of-bounds", rdx->loc, 2U, 1U);
       });
    // an invariant reading m[0][j] unguarded where j runs to 3
    lc("e1e2e_loop_matrix_invariant_second_index_fail",
       "the invariant m[0][j] == 0 with j running to 3: the second index against the row's length fails in the invariant (out-of-bounds at the read)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, true)};
           Var* j {k.local(f, "j", k.i32)};
           const Expr* rd {mread(k, m, N(k, "0"), k.v(j))};
           const Stmt* loop {
               k.whl(k.lt(k.v(j), N(k, "3")),
                     {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3")))), k.cl(k.eq(rd, N(k, "0")))},
                     {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc({inc(k, j)}))};
           f->body = k.blkc({k.decl(m, nullptr), k.decl(j, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
           // two rows at the read (the constant first index has no row; the second index at the entry and at the back edge): the back edge, where j is 3, is the one that fails
           k.fail_only("out-of-bounds", rd->loc, 2U, 1U, "whose second index may be outside");
       });
    lc("e1e2e_loop_matrix_invariant_first_index_fail",
       "the invariant m[i][0] == 0 with i running to 2: the first index against the matrix fails in the invariant (out-of-bounds at the read)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, true)};
           Var* i {k.local(f, "i", k.i32)};
           const Expr* rd {mread(k, m, k.v(i), N(k, "0"))};
           const Stmt* loop {
               k.whl(k.lt(k.v(i), N(k, "2")),
                     {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2")))), k.cl(k.eq(rd, N(k, "0")))},
                     {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc({inc(k, i)}))};
           f->body = k.blkc({k.decl(m, nullptr), k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
           // two rows at the read (the constant second index has no row): the first index at the back edge, where i is 2, fails
           k.fail_only("out-of-bounds", rd->loc, 2U, 1U, "whose first index may be outside");
       });
    lc("e1e2e_loop_matrix_invariant_uninitialized_fail",
       "int m[2][3]; with no zero fill and the invariant m[0][0] == 0: the cell is read before it is given a value (uninitialized in the invariant)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, false)};
           Var* j {k.local(f, "j", k.i32)};
           const Expr* rd {mread(k, m, N(k, "0"), N(k, "0"))};
           const Clause want {k.cl(k.eq(rd, N(k, "0")))};
           const Stmt* loop {k.whl(k.lt(k.v(j), N(k, "3")),
                                   {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3")))), want},
                                   {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc({inc(k, j)}))};
           f->body = k.blkc({k.decl(m, nullptr), k.decl(j, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
           // two rows at the read (the entry and the back edge): the entry one fails (Silicon reports one failure for the loop's entry)
           k.fail_only("uninitialized", rd->loc, 2U, 0U);
           (void)want;
       });

    // ---- a row parameter's range ----
    auto rplace = [](Kit& k, Function* f, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.pv(f, 0), i, k.at()), j, k.at());
    };
    auto rread = [=](Kit& k, Function* f, const Expr* i, const Expr* j)
    {
        return k.deref(rplace(k, f, i, j));
    };
    // the contract read a[0][0] of the formal
    auto c00 = [=](Kit& k, Function* f)
    {
        const Expr* row {k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at())};
        return k.deref(k.b.elem_addr(row, N(k, "0"), k.at()));
    };
    auto atom = [=](Kit& k, Function* f, AtomKind kind, int amount)
    {
        return k.b.range_atom(kind, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    // int f(int (*a)[3], int n) requires(<kind>(a, n) && n > 0 && a[0][0] == 7) ensures(<kind>(a, n) && r == 7)
    //   { int i = 0; int j = 0; int t = 0;
    //     while (i < n) inv 0 <= i <= n && a[0][0] == 7 decreases n - i { j = 0; while (j < 3) inv 0 <= j <= 3 && (j >= 3 || a[i][j] == a[i][j]) && a[0][0] == 7 decreases 3 - j { t = a[i][j]; j++; } i++; }
    //     check(a[0][0] == <want>); return a[0][0]; }
    // `mode`: 0 the reads, 1 want 8, 2 check(false) after, 3 check(false) at the end of the inner body; `owns`: owns_range, else reads_range
    const auto lsum = [=](Kit& k, bool owns, int mode) -> const Stmt*
    {
        Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        k.req(f,
              k.land(k.land(atom(k, f, kind, 0), k.land(k.gt(fx(k, f, 1), N(k, "0")), k.lt(fx(k, f, 1), N(k, "100")))),
                     k.eq(c00(k, f), N(k, "7"))));
        k.ens(f, k.land(atom(k, f, kind, 0), k.eq(k.R(f), N(k, "7"))));
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        Var* t {k.local(f, "t", k.i32)};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib {k.asg(t, rread(k, f, k.v(i), k.v(j)))};
        if (c3 != nullptr)
        {
            ib.push_back(c3);
        }
        ib.push_back(inc(k, j));
        const Expr* same {k.eq(rread(k, f, k.v(i), k.v(j)), rread(k, f, k.v(i), k.v(j)))};
        const Stmt* inner {k.whl(k.lt(k.v(j), N(k, "3")),
                                 {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3")))),
                                  k.cl(k.lor(k.ge(k.v(j), N(k, "3")), same)),
                                  k.cl(k.eq(rread(k, f, N(k, "0"), N(k, "0")), N(k, "7")))},
                                 {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc(ib))};
        const Stmt* outer {k.whl(k.lt(k.v(i), k.pv(f, 1)),
                                 {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), k.pv(f, 1)))),
                                  k.cl(k.eq(rread(k, f, N(k, "0"), N(k, "0")), N(k, "7")))},
                                 {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({k.asg(j, N(k, "0")), inner, inc(k, i)}))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        const Stmt* c1 {k.chk(k.eq(rread(k, f, N(k, "0"), N(k, "0")), N(k, mode == 1 ? "8" : "7")))};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), k.decl(j, N(k, "0")), k.decl(t, N(k, "0")), outer, c1};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        body.push_back(k.ret(rread(k, f, N(k, "0"), N(k, "0"))));
        f->body = k.blkc(std::move(body));
        k.done(f);
        return mode == 1 ? c1 : mode == 2 ? c2 : mode == 3 ? c3 : nullptr;
    };
    lc("e1e2e_loop_rowrange_nested_reads",
       "two nested loops over reads_range(a, n) of rows that only read a[i][j]: one range place for the rows (cells over two bound variables, the atom's amount), the invariants read a[0][0] and a[i][j] framed by it, the value 7 is kept",
       [=](Kit& k)
       {
           lsum(k, false, 0);
       });
    lc("e1e2e_loop_rowrange_nested_reads_owns",
       "the same over owns_range(a, n): the loops write nothing, so the values are kept",
       [=](Kit& k)
       {
           lsum(k, true, 0);
       });
    lc("e1e2e_loop_rowrange_nested_wrong_fail", "the same loops, then check(a[0][0] == 8): fails at the check",
       [=](Kit& k)
       {
           k.fail("check", lsum(k, false, 1)->loc);
       });
    lc("e1e2e_loop_rowrange_nested_vacuity", "the same loops with check(false) after them: it fails",
       [=](Kit& k)
       {
           k.fail("check", lsum(k, false, 2)->loc);
       });
    lc("e1e2e_loop_rowrange_nested_body_vacuity",
       "the same loops with check(false) at the end of the inner body: it fails",
       [=](Kit& k)
       {
           k.fail("check", lsum(k, false, 3)->loc);
       });
    // void zero(int (*a)[3], int n) requires(owns_range(a, n) && n > 0 && n < 100) ensures(owns_range(a, n))
    //   { int i = 0; int j = 0; while (i < [n|=n]) inv 0 <= i <= n decreases n - i { j = 0; while (j < 3) inv 0 <= j <= 3 decreases 3 - j { a[i][j] = 0; j++; } i++; } }
    // `mode`: 0 the fill, 1 i <= n, 2 check(false) after, 3 check(false) at the end of the inner body
    const auto lfill = [=](Kit& k, int mode, SrcLoc* inv_loc = nullptr) -> const Stmt*
    {
        Function* f {k.fn("zero", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
        k.req(f, k.land(atom(k, f, AtomKind::OwnsRange, 0),
                        k.land(k.gt(fx(k, f, 1), N(k, "0")), k.lt(fx(k, f, 1), N(k, "100")))));
        k.ens(f, atom(k, f, AtomKind::OwnsRange, 0));
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Stmt* w {k.store(rplace(k, f, k.v(i), k.v(j)), N(k, "0"))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib {w};
        if (c3 != nullptr)
        {
            ib.push_back(c3);
        }
        ib.push_back(inc(k, j));
        const Stmt* inner {k.whl(k.lt(k.v(j), N(k, "3")),
                                 {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3"))))},
                                 {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc(ib))};
        const Expr* cond {mode == 1 ? k.le(k.v(i), k.pv(f, 1)) : k.lt(k.v(i), k.pv(f, 1))};
        const Clause ibound {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), k.pv(f, 1))))};
        if (inv_loc != nullptr)
        {
            *inv_loc = ibound.loc;
        }
        const Stmt* outer {
            k.whl(cond, {ibound}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({k.asg(j, N(k, "0")), inner, inc(k, i)}))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), k.decl(j, N(k, "0")), outer};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blkc(std::move(body));
        k.done(f);
        return mode == 2 ? c2 : mode == 3 ? c3 : w;
    };
    lc("e1e2e_loop_rowrange_fill",
       "two nested loops write a[i][j] = 0 over owns_range(a, n) of rows: the range place carries the permissions, the flags and the type's range through both loops, so the ensures owns_range holds",
       [=](Kit& k)
       {
           lfill(k, 0);
       });
    lc("e1e2e_loop_rowrange_fill_high",
       "the outer loop runs to i <= n: a[n][j] fails its first bound row and its write-permission row at the store",
       [=](Kit& k)
       {
           SrcLoc inv;
           const Stmt* w {lfill(k, 1, &inv)};
           // a[n] one past the range: both bound rows at the store fail (the row's length is unknown there) and so does the write permission; the invariant i <= n is not preserved either
           k.fail("out-of-bounds", w->loc, 2U);
           k.fail("permission", w->loc);
           k.fail("invariant-preserved", inv);
       });
    lc("e1e2e_loop_rowrange_fill_vacuity", "the fill with check(false) after the loops: it fails",
       [=](Kit& k)
       {
           k.fail("check", lfill(k, 2)->loc);
       });
    lc("e1e2e_loop_rowrange_fill_body_vacuity", "the fill with check(false) at the end of the inner body: it fails",
       [=](Kit& k)
       {
           k.fail("check", lfill(k, 3)->loc);
       });

    // bool zero_at(int k, const int *p, int m) { return !(0 <= k && k < m) || p[k] == 0; }, `forall zero_at(p, m)`
    auto mkq = [=](Kit& k, int id, const std::vector<const Expr*>& args)
    {
        const Expr* ib {k.b.bound(id, k.i32.int_type, "i", k.at())};
        std::vector<const Expr*> a;
        const std::vector<Type> ptypes {k.ptr(k.i32), k.i32};
        for (size_t j {0U}; j < args.size(); ++j)
        {
            a.push_back(k.b.forall_arg(id, static_cast<int>(j), ptypes[j],
                                       args[j]->c_text.empty() ? "x" : args[j]->c_text, k.at()));
        }
        const Expr* elem {k.deref(k.b.elem_addr(a[0], ib, k.at()))};
        const Expr* b {k.lor(k.lnot(k.land(k.ge_c(ib, k.i32, "0"), k.lt(ib, a[1]))), k.eq(elem, N(k, "0")))};
        return k.b.forall(id, k.i32.int_type, b, args, "forall(zero_at)", "zero_at", k.at());
    };
    // ---- a callee over rows called inside a loop (trap 3: a callee's write through a range over rows reaches the cells' field) ----
    // void w(int (*b)[3], int n) requires(<kind>(b, n) && n > 0) ensures(<kind>(b, n))   (no body)
    auto mkw = [=](Kit& k, bool owns) -> Function*
    {
        Function* w {k.fn("w", k.vd, {{"b", rowptr(k)}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        k.req(w, k.land(k.b.range_atom(kind, fx(k, w, 0), fx(k, w, 1), 0, k.at()), k.gt(fx(k, w, 1), N(k, "0"))));
        k.ens(w, k.b.range_atom(kind, fx(k, w, 0), fx(k, w, 1), 0, k.at()));
        k.trusted(w);
        return w;
    };
    // int f(void) { int m[2][3] = {0}; int i = 0; while (i < 2) inv 0 <= i <= 2 && m[1][2] == 0 decreases 2 - i { w(m, 2); i++; } return 0; }
    const auto lcall_m = [=](Kit& k, bool owns) -> const Clause
    {
        Function* w {mkw(k, owns)};
        Function* f {k.fn("f", k.i32)};
        Var* m {mat(k, f, true)};
        Var* i {k.local(f, "i", k.i32)};
        const Clause keep {k.cl(k.eq(mread(k, m, N(k, "1"), N(k, "2")), N(k, "0")))};
        const Expr* arg {k.b.elem_addr(k.addr(m), N(k, "0"), k.at())};
        const Stmt* loop {
            k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2")))), keep},
                  {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc({k.es(k.call(w, {arg, N(k, "2")})), inc(k, i)}))};
        f->body = k.blkc({k.decl(m, nullptr), k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return keep;
    };
    lc("e1e2e_loop_matrix_calls_a_reading_callee",
       "the loop calls w(m, 2) with reads_range over rows: the callee writes nothing, m[1][2] == 0 is kept",
       [=](Kit& k)
       {
           lcall_m(k, false);
       });
    lc("e1e2e_loop_matrix_calls_an_owning_callee_fail",
       "the loop calls w(m, 2) with owns_range over rows: the callee may write the matrix's cells, so m[1][2] == 0 is not kept (invariant fails at the back edge, once)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", lcall_m(k, true).loc);
       });
    // int f(int (*a)[3], int n) requires(owns_range(a, n) && n > 0 && a[0][0] == 7) ensures(owns_range(a, n)) { int i = 0; while (i < 1) inv 0 <= i <= 1 && a[0][0] == 7 decreases 1 - i { w(a, n); i++; } }
    const auto lcall_r = [=](Kit& k, bool owns) -> const Clause
    {
        Function* w {mkw(k, owns)};
        Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                               k.gt(fx(k, f, 1), N(k, "0"))),
                        k.eq(c00(k, f), N(k, "7"))));
        k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        const Clause keep {k.cl(k.eq(rread(k, f, N(k, "0"), N(k, "0")), N(k, "7")))};
        const Stmt* loop {
            k.whl(k.lt(k.v(i), N(k, "1")), {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "1")))), keep},
                  {k.cl(k.sub(N(k, "1"), k.v(i)))}, k.blkc({k.es(k.call(w, {k.pv(f, 0), k.pv(f, 1)})), inc(k, i)}))};
        f->body = k.blkc({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return keep;
    };
    lc("e1e2e_loop_rowrange_calls_a_reading_callee",
       "the loop forwards (a, n) to w with reads_range over rows: a[0][0] == 7 is kept",
       [=](Kit& k)
       {
           lcall_r(k, false);
       });
    lc("e1e2e_loop_rowrange_calls_an_owning_callee_fail",
       "the loop forwards (a, n) to w with owns_range over rows: a[0][0] == 7 is not kept (invariant fails at the back edge, once)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", lcall_r(k, true).loc);
       });
    // void f(int (*a)[3]) { int i = 0; while (i < 1) inv 0 <= i <= 1 && a[0][0] == 0 decreases 1 - i { i++; } }  with no atom on `a`
    lc("e1e2e_loop_rowparam_invariant_unframed_fail",
       "a row parameter with no atom: its element in a loop invariant is held by no place (permission at the read in the clause)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", rowptr(k)}})};
           Var* i {k.local(f, "i", k.i32)};
           const Expr* rd {rread(k, f, N(k, "0"), N(k, "0"))};
           const Stmt* loop {
               k.whl(k.lt(k.v(i), N(k, "1")),
                     {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "1")))), k.cl(k.eq(rd, N(k, "0")))},
                     {k.cl(k.sub(N(k, "1"), k.v(i)))}, k.blkc({inc(k, i)}))};
           f->body = k.blkc({k.decl(i, N(k, "0")), loop});
           k.done(f);
           k.fail_only("permission", rd->loc, 2U, 0U, "this loop invariant reads");
       });
    // a quantified read beside a matrix place: int f(void) { int m[2][3] = {0}; int p[3] = {0}; int i = 0; while (i < 3) inv 0 <= i <= 3 && forall zero_at(p, 3) decreases 3 - i { m[0][i] = 1; i++; } }
    lc("e1e2e_loop_matrix_forall_beside_a_scalar_array",
       "the loop writes m[0][i] and keeps forall zero_at(p, 3): p[j] under the binder is framed by p's array place; the matrix's place is left out of the binder's framing (no computed loc term)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           Var* m {mat(k, f, true)};
           Var* p {f->new_local("p", k.prog.array_of(k.i32, 3U), k.at())};
           f->mark_address_taken(p);
           p->zero_filled = true;
           Var* i {k.local(f, "i", k.i32)};
           const Expr* q {mkq(k, 0, {k.addr(p), N(k, "3")})};
           const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")),
                                   {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), k.cl(q)},
                                   {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                   k.blkc({k.store(mplace(k, m, N(k, "0"), k.v(i)), N(k, "1")), inc(k, i)}))};
           f->body = k.blkc({k.decl(m, nullptr), k.decl(p, nullptr), k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
       });

    // ---- slice 2f (question 367): the tie of a row parameter's contract element across the two states ----
    // int f(int (*a)[3], int n) requires(<kind>(a, n) && n > 1 && a[<ri>][<rj>] != 0) ensures(<kind>(a, n) && r == 10 / a[<ei>][<ej>]) { return 10 / a[0][0]; } (the body reads a[0][0] when it divides by a cell the contract names)
    const auto ltie = [=](Kit& k, bool owns, const char* ri, const char* rj, const char* ei,
                          const char* ej) -> const Expr*
    {
        Function* f {k.fn("f", k.i32, {{"a", rowptr(k)}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto cr = [&](const char* i, const char* j)
        {
            const Expr* row {k.b.elem_addr(fx(k, f, 0), N(k, i), k.at())};
            return k.deref(k.b.elem_addr(row, N(k, j), k.at()));
        };
        k.req(f, k.land(k.land(k.b.range_atom(kind, fx(k, f, 0), fx(k, f, 1), 0, k.at()), k.gt(fx(k, f, 1), N(k, "1"))),
                        k.ne(cr(ri, rj), N(k, "0"))));
        const bool same {std::string(ri) == ei && std::string(rj) == ej};
        const Expr* d {k.div(N(k, "10"), cr(ei, ej))};
        // the same cell: r == 10 / a[e]; another cell: r == 10 / a[r] (tied) and the division by a[e] beside it, which has no entry ghost to be tied to
        const Expr* post {same ? k.eq(k.R(f), d) : k.land(k.eq(k.R(f), k.div(N(k, "10"), cr(ri, rj))), k.eq(d, d))};
        k.ens(f, k.land(k.b.range_atom(kind, fx(k, f, 0), fx(k, f, 1), 0, k.at()), post));
        f->body = k.blk({k.ret(k.div(N(k, "10"), rread(k, f, N(k, ri), N(k, rj))))});
        k.done(f);
        return d;
    };
    lc("e1e2e_rowparam_wf_tie_reads",
       "requires reads_range(a, n) && a[0][0] != 0, ensures r == 10 / a[0][0]: the exit ghost of a[0][0] equals its entry ghost under the reads site (the tie), so the division is by a nonzero",
       [=](Kit& k)
       {
           ltie(k, false, "0", "0", "0", "0");
       });
    lc("e1e2e_rowparam_wf_tie_owns_fail",
       "the same under owns_range: the function may write the cell, no tie, the ensures' division is not proved (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail("division-by-zero", ltie(k, true, "0", "0", "0", "0")->loc);
       });
    lc("e1e2e_rowparam_wf_tie_other_cell_fail",
       "requires a[0][1] != 0 and ensures dividing by a[1][1] under reads_range: another cell, its ghost is not tied to a[0][1]'s (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail_only("division-by-zero", ltie(k, false, "0", "1", "1", "1")->loc, 2U, 0U);
       });
    lc("e1e2e_rowparam_wf_tie_other_column_fail",
       "requires a[1][0] != 0 and ensures dividing by a[1][1]: the same row, another column (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail_only("division-by-zero", ltie(k, false, "1", "0", "1", "1")->loc, 2U, 0U);
       });
    lc("e1e2e_rowparam_wf_tie_other_row_fail",
       "requires a[0][1] != 0 and ensures dividing by a[1][1]... through a different first index and the same column: the first index is part of the key (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail_only("division-by-zero", ltie(k, false, "0", "2", "1", "2")->loc, 2U, 0U);
       });
    lc("e1e2e_rowparam_wf_tie_other_cell_swapped_fail",
       "requires a[0][1] != 0 and ensures dividing by a[1][0]: both indexes differ (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail_only("division-by-zero", ltie(k, false, "0", "1", "1", "0")->loc, 2U, 0U);
       });
}

// ---- decision 0064 items 3, 9 and 13 (Part E, slice 3a): a global or `static` local MATRIX of scalars ----
void define_part_e_global_matrix_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // int gm[2][3] with the rows listed: `rows` one vector of cell values per listed row (a row's rest is zero), `unknown` the external linkage of C++ (every cell known to be in range only)
    auto gmat = [=](Kit& k, const char* name, const std::vector<std::vector<const char*>>& rows, bool unknown = false,
                    const Function* owner = nullptr) -> Global*
    {
        Global* g {k.prog.add_global(name, e1e2e_matrix(k), k.at(), owner)};
        for (const auto& r : rows)
        {
            GlobalLeaf row;
            row.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
            row.rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
            row.loc = k.at();
            for (const char* v : r)
            {
                GlobalLeaf c;
                c.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
                c.value = v;
                c.init = k.num(k.i32, v);
                c.loc = k.at();
                row.elems.push_back(c);
            }
            g->elems.push_back(row);
        }
        g->rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
        return g;
    };
    auto gplace = [](Kit& k, const Global* g, const char* i, const char* j)
    {
        const Expr* row {k.b.elem_addr(k.b.global_addr(g, k.at()), k.num(k.i32, i), k.at())};
        return k.b.elem_addr(row, k.num(k.i32, j), k.at());
    };
    auto gread = [=](Kit& k, const Global* g, const char* i, const char* j)
    {
        return k.deref(gplace(k, g, i, j));
    };
    auto gwrite = [=](Kit& k, const Global* g, const char* i, const char* j, const char* v)
    {
        return k.store(gplace(k, g, i, j), N(k, v));
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the global's rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto main_end = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
        body.push_back(k.ret(k.num(k.i32, "0")));
    };
    // int gm[2][3] = {{1, 2, 3}, {4}}; int main() { check(gm[0][0] == 1 && ... && gm[1][0] == 4 && gm[1][1] == 0 && gm[1][2] == 0); gm[1][1] = 7; check(gm[1][1] == 7 && gm[1][0] == 4 && gm[0][1] == 2); }
    lcv("e1e2e_gmat_main_read_write",
        "main holds gm[2][3] = {{1, 2, 3}, {4}} from program_start: the listed cells, the cells of row 1 its list leaves out (zero), then a write and a read back",
        [=](Kit& k)
        {
            Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
            Function* m {k.fn("main", k.i32)};
            auto eq = [&](const char* i, const char* j, const char* v)
            {
                return k.eq_c(gread(k, g, i, j), k.i32, v);
            };
            const Stmt* c1 {k.chk(k.land(
                k.land(k.land(eq("0", "0", "1"), eq("0", "1", "2")), k.land(eq("0", "2", "3"), eq("1", "0", "4"))),
                k.land(eq("1", "1", "0"), eq("1", "2", "0"))))};
            const Stmt* w {gwrite(k, g, "1", "1", "7")};
            const Stmt* c2 {k.chk(k.land(eq("1", "1", "7"), k.land(eq("1", "0", "4"), eq("0", "1", "2"))))};
            std::vector<const Stmt*> body {c1, w, c2};
            main_end(k, body);
            m->body = k.blk(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    // int gm[2][3] = {{1, 2, 3}}; main: the rows the list does not reach are zero
    lc("e1e2e_gmat_zero_rows",
       "gm[2][3] = {{1, 2, 3}}: row 1 is not reached by the list, every cell of it is zero (one quantifier over both binders)",
       [=](Kit& k)
       {
           Global* g {gmat(k, "gm", {{"1", "2", "3"}})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(
               k.land(k.eq_c(gread(k, g, "1", "0"), k.i32, "0"),
                      k.land(k.eq_c(gread(k, g, "1", "2"), k.i32, "0"), k.eq_c(gread(k, g, "0", "2"), k.i32, "3"))))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    lc("e1e2e_gmat_zero_rows_wrong_fail",
       "the same checking gm[1][2] == 1 (a cell of a row the list does not reach): fails",
       [=](Kit& k)
       {
           Global* g {gmat(k, "gm", {{"1", "2", "3"}})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(gread(k, g, "1", "2"), k.i32, "1"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2e_gmat_listed_cell_wrong_fail",
       "the list's cell gm[1][0] is 4: the check that gm[1][0] == 1 (the value of gm[0][0]) fails",
       [=](Kit& k)
       {
           Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(gread(k, g, "1", "0"), k.i32, "1"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2e_gmat_left_out_cell_wrong_fail",
       "row 1's list is {4}: the check that gm[1][1] == 4 (the same row, another column) fails",
       [=](Kit& k)
       {
           Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(gread(k, g, "1", "1"), k.i32, "4"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2e_gmat_unknown_value_fail",
       "C++ external linkage: every cell holds some value of its type, none known: gm[0][0] == 0 fails, a write then a read back verifies",
       [=](Kit& k)
       {
           Global* g {gmat(k, "gm", {}, true)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* w {gwrite(k, g, "0", "0", "5")};
           const Stmt* ok {k.chk(k.eq_c(gread(k, g, "0", "0"), k.i32, "5"))};
           const Stmt* c {k.chk(k.eq_c(gread(k, g, "1", "2"), k.i32, "0"))};
           m->body = k.blk({w, ok, c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    // int gm[2][3] = {{1, 2, 3}, {4}}; int main() { int i = 0; while (i < 3) inv 0 <= i <= 3 && gm[1][0] == 4 && (i >= 3 || gm[0][i] >= 1) decreases 3 - i { [gm[0][i] = 9;] i++; } check(gm[0][1] == <want>); return 0; }
    // `mode`: 0 the reads, 1 the loop writes gm[0][i] (the invariant gm[1][0] == 4 is not kept), 2 want 3 after the loop
    const auto lmain = [=](Kit& k, int mode) -> SrcLoc
    {
        Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
        Function* f {k.fn("main", k.i32)};
        Var* i {k.local(f, "i", k.i32)};
        const Clause keep {k.cl(k.eq_c(gread(k, g, "1", "0"), k.i32, "4"))};
        const Expr* rd {
            k.deref(k.b.elem_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "0"), k.at()), k.v(i), k.at()))};
        std::vector<const Stmt*> bs;
        if (mode == 1)
        {
            bs.push_back(
                k.store(k.b.elem_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "0"), k.at()), k.v(i), k.at()),
                        N(k, "9")));
        }
        bs.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        std::vector<Clause> invs {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep};
        if (mode != 1)
        {
            invs.push_back(k.cl(k.lor(k.ge(k.v(i), N(k, "3")), k.ge_c(rd, k.i32, "1"))));
        }
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")), invs, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs))};
        const Stmt* c {k.chk(k.eq_c(gread(k, g, "0", "1"), k.i32, mode == 2 ? "3" : "2"))};
        f->body = k.blkc({k.decl(i, N(k, "0")), loop, c, k.ret(N(k, "0"))});
        k.done(f);
        k.prog.set_entry(f);
        return c->loc;
    };
    lc("e1e2e_gmat_loop_main_reads",
       "main loops over gm with invariants that read gm[1][0] and gm[0][i] (both bounds, the global's array place frames them): the values are kept",
       [=](Kit& k)
       {
           lmain(k, 0);
       });
    lc("e1e2e_gmat_loop_main_wrong_fail", "the same loop, then check(gm[0][1] == 3): fails at the check",
       [=](Kit& k)
       {
           k.fail("check", lmain(k, 2));
       });
    lc("e1e2e_gmat_loop_main_write_loses_values_fail",
       "the loop writes gm[0][i] by name: the global's place is reached, so gm[0][1] == 2 (an invariant mentions only gm[1][0], which the write cannot change) is not kept after the loop: the check fails",
       [=](Kit& k)
       {
           k.fail("check", lmain(k, 1));
       });
    auto gatom = [](Kit& k, AtomKind kind, const Global* g, int amount, bool implicit = false)
    {
        return k.b.global_atom(kind, k.b.global_addr(g, k.at()), amount, k.at(), implicit);
    };
    // void put(void) requires(owns(gm)) ensures(owns(gm)) { gm[1][1] = 9; check(gm[1][1] == 9); }   int main() { put(); int z = gm[0][1]; return 0; }
    const auto put_main = [=](Kit& k, bool with_atom, bool reads) -> const Stmt*
    {
        Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
        Function* f {k.fn("put", k.vd)};
        const AtomKind kind {reads ? AtomKind::Reads : AtomKind::Owns};
        if (with_atom)
        {
            k.req(f, gatom(k, kind, g, 0));
            k.ens(f, gatom(k, kind, g, 0));
        }
        const Stmt* w {gwrite(k, g, "1", "1", "9")};
        f->body = k.blk({w, k.chk(k.eq_c(gread(k, g, "1", "1"), k.i32, "9"))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* z {k.local(m, "z", k.i32)};
        std::vector<const Stmt*> body {k.es(k.call(f, {})), k.decl(z, gread(k, g, "0", "1"))};
        main_end(k, body);
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return w;
    };
    lcv("e1e2e_gmat_owns_callee",
        "put() takes owns(gm) (the whole-array atom over rows: every cell of every row): it writes gm[1][1] and reads it back; main holds gm from program_start and reads gm[0][1] after the call",
        [=](Kit& k)
        {
            put_main(k, true, false);
        });
    lc("e1e2e_gmat_unowned_write_fail",
       "the same callee with no atom on gm: the write gm[1][1] = 9 is a permission error at the store",
       [=](Kit& k)
       {
           const Stmt* w {put_main(k, false, false)};
           k.fail_only("permission", w->loc, 1U, 0U);
           k.fail("check", w->loc, 0U);
       });
    lc("e1e2e_gmat_reads_callee_write_fail",
       "put() with reads(gm) only: the write is a permission error at the store (the atom gives a fraction)",
       [=](Kit& k)
       {
           const Stmt* w {put_main(k, true, true)};
           k.fail("permission", w->loc);
       });
    // void zero(void) requires(owns(gm)) ensures(owns(gm)) { int i = 0; int j = 0; while (i < 2) inv 0 <= i <= 2 && [gm[1][0] == gm[1][0]] decreases 2 - i { j = 0; while (j < 3) inv 0 <= j <= 3 decreases 3 - j { gm[i][j] = 0; j++; } i++; } }
    // int main() { zero(); return 0; }   `mode`: 0 the fill, 1 no atom on gm (the loop is unframed), 2 the invariant reads gm[1][0] (framed by the held atom)
    const auto lzero = [=](Kit& k, int mode) -> const Expr*
    {
        Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
        Function* f {k.fn("zero", k.vd)};
        if (mode != 1)
        {
            k.req(f, gatom(k, AtomKind::Owns, g, 0));
            k.ens(f, gatom(k, AtomKind::Owns, g, 0));
        }
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Expr* rd {gread(k, g, "1", "0")};
        const Stmt* inner {k.whl(
            k.lt(k.v(j), N(k, "3")), {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3"))))},
            {k.cl(k.sub(N(k, "3"), k.v(j)))},
            k.blkc({k.store(k.b.elem_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), k.v(i), k.at()), k.v(j), k.at()),
                            N(k, "0")),
                    k.asg(j, k.add(k.v(j), N(k, "1")))}))};
        std::vector<Clause> invs {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2"))))};
        if (mode != 0)
        {
            invs.push_back(k.cl(k.eq(rd, rd)));
        }
        const Stmt* outer {k.whl(k.lt(k.v(i), N(k, "2")), invs, {k.cl(k.sub(N(k, "2"), k.v(i)))},
                                 k.blkc({k.asg(j, N(k, "0")), inner, k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        f->body = k.blkc({k.decl(i, N(k, "0")), k.decl(j, N(k, "0")), outer});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return rd;
    };
    lc("e1e2e_gmat_loop_owns_fill",
       "zero() holds owns(gm) and fills gm[i][j] = 0 in two nested loops: the range place over rows (the whole-array atom) carries the permissions through both",
       [=](Kit& k)
       {
           lzero(k, 0);
       });
    lc("e1e2e_gmat_loop_owns_invariant_read",
       "the same with the invariant gm[1][0] == gm[1][0] in the outer loop: the read is framed by the range place after its two bound rows",
       [=](Kit& k)
       {
           lzero(k, 2);
       });
    lc("e1e2e_gmat_loop_unowned_fail",
       "the same loop with no atom on gm: the read in the invariant is held by no place (permission at the read in the clause)",
       [=](Kit& k)
       {
           const Expr* rd {lzero(k, 1)};
           // four rows at the read (the outer loop's framing assert at its entry and its back edge, and the inner loop's, which stands in the outer body): the first fails and ends the path
           k.fail_only("permission", rd->loc, 4U, 0U, "this loop invariant reads");
       });
    // `static int sm[2][3]` of f with the implicit owns(sm): void f(void) requires(owns(sm)) ensures(owns(sm)) { sm[1][1] = 9; check(sm[1][1] == 9); }  int main() { f(); return 0; }
    const auto lstatic = [=](Kit& k, bool with_atom) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Global* g {gmat(k, "sm", {}, false, f)};
        if (with_atom)
        {
            f->requires_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
            f->ensures_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
        }
        const Stmt* w {gwrite(k, g, "1", "1", "9")};
        f->body = k.blk({w, k.chk(k.eq_c(gread(k, g, "1", "1"), k.i32, "9"))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return w;
    };
    lc("e1e2e_gmat_static_local",
       "a `static` local matrix with its implicit owns atom: f writes sm[1][1] and reads it back",
       [=](Kit& k)
       {
           lstatic(k, true);
       });
    lc("e1e2e_gmat_static_local_unowned_fail", "the same with no atom on sm: the write is a permission error",
       [=](Kit& k)
       {
           k.fail("permission", lstatic(k, false)->loc);
       });
    // ---- sequencing (record 0064 item 11): a call that holds gm whole beside an access or another call in one full expression ----
    // int put(void) requires(owns(gm)) ensures(owns(gm)) { return 1; }   int main() { int z = put() + <rhs>; return 0; }
    const auto lseq = [=](Kit& k, int mode) -> const Expr*
    {
        Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
        Function* put {k.fn("put", k.i32)};
        k.req(put, gatom(k, AtomKind::Owns, g, 0));
        k.ens(put, k.land(gatom(k, AtomKind::Owns, g, 0), k.eq(k.R(put), N(k, "1"))));
        put->body = k.blk({k.ret(N(k, "1"))});
        k.done(put);
        Function* m {k.fn("main", k.i32)};
        Var* w {k.local(m, "w", k.i32)};
        Var* z {k.local(m, "z", k.i32)};
        const Expr* callx {k.call(put, {})};
        // mode 0: beside a cell of gm (overlap), 1: beside a plain local (disjoint), 2: beside a second call (overlap)
        const Expr* rhs {mode == 0 ? gread(k, g, "1", "2") : mode == 1 ? k.v(w) : k.call(put, {})};
        m->body = k.blkc({k.decl(w, N(k, "1")), k.decl(z, k.add(callx, rhs)), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return rhs;
    };
    lc("e1e2e_gmat_seq_disjoint",
       "put() + w with w a plain local: the call holds gm, the local is another object: no row fails",
       [=](Kit& k)
       {
           lseq(k, 1);
       });
    lc("e1e2e_gmat_seq_call_and_cell_overlap_fail",
       "put() + gm[1][2]: the call holds every cell of gm and the read names one of them (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", lseq(k, 0)->loc);
       });
    lc("e1e2e_gmat_seq_two_calls_overlap_fail", "put() + put(): two calls that both hold gm (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", lseq(k, 2)->loc);
       });
    // ---- what slice 3a leaves refused: the contract read gm[e][e2] (question 362: built in 3a's last step if at all), a global matrix given to a row parameter ----
    // ---- the contract read gm[e][e2] of a global matrix (question 362), framed by owns(gm) / reads(gm) ----
    // int f(void) requires(<kind>(gm) && gm[<ri>][<rj>] != 0) ensures(<kind>(gm) && r == 10 / gm[<ei>][<ej>]) { return 10 / gm[<ri>][<rj>]; }   int main() { return f(); }
    const auto lgread = [=](Kit& k, bool with_atom, bool reads, const char* ri, const char* rj, const char* ei,
                            const char* ej, SrcLoc* call_loc = nullptr, SrcLoc* fn_loc = nullptr) -> const Expr*
    {
        Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
        Function* f {k.fn("f", k.i32)};

        const AtomKind kind {reads ? AtomKind::Reads : AtomKind::Owns};
        const Expr* rd1 {gread(k, g, ri, rj)};
        if (fn_loc != nullptr)
        {
            *fn_loc = rd1->loc;
        }
        const Expr* pre {k.ne(rd1, N(k, "0"))};
        const Expr* d {k.div(N(k, "10"), gread(k, g, ei, ej))};
        const bool same {std::string(ri) == ei && std::string(rj) == ej};
        const Expr* post {same ? k.eq(k.R(f), d)
                               : k.land(k.eq(k.R(f), k.div(N(k, "10"), gread(k, g, ri, rj))), k.eq(d, d))};
        k.req(f, with_atom ? k.land(gatom(k, kind, g, 0), pre) : pre);
        k.ens(f, with_atom ? k.land(gatom(k, kind, g, 0), post) : post);
        f->body = k.blk({k.ret(k.div(N(k, "10"), gread(k, g, ri, rj)))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(f, {})};
        if (call_loc != nullptr)
        {
            *call_loc = callx->loc;
        }
        m->body = k.blk({k.ret(callx)});
        k.done(m);
        k.prog.set_entry(m);
        return d;
    };
    lc("e1e2e_gmat_contract_read_tied",
       "requires reads(gm) && gm[1][0] != 0, ensures r == 10 / gm[1][0]: framed by the whole-array atom, tied across the two states under reads, so the division is by a nonzero; main's gm[1][0] is 4 from program_start",
       [=](Kit& k)
       {
           lgread(k, true, true, "1", "0", "1", "0");
       });
    lc("e1e2e_gmat_contract_read_owns_fail",
       "the same under owns(gm): the function may write the cell, no tie, the ensures' division is not proved (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail("division-by-zero", lgread(k, true, false, "1", "0", "1", "0")->loc);
       });
    lc("e1e2e_gmat_contract_read_other_cell_fail",
       "requires gm[1][0] != 0 and ensures dividing by gm[1][1] under reads: another cell of the same row has its own ghost (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail_only("division-by-zero", lgread(k, true, true, "1", "0", "1", "1")->loc, 2U, 0U);
       });
    lc("e1e2e_gmat_contract_read_other_row_fail",
       "requires gm[0][1] != 0 and ensures dividing by gm[1][1]: another row, the same column (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail_only("division-by-zero", lgread(k, true, true, "0", "1", "1", "1")->loc, 2U, 0U);
       });
    lc("e1e2e_gmat_contract_read_unframed_fail",
       "the same with no atom on gm: the read gm[1][0] in the requires is held by nothing (permission at the quantified read)",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc fn;
           lgread(k, false, false, "1", "0", "1", "0", &call, &fn);
           // the read yields the dummy 0 where nothing frames it: the precondition fails at main's call, and the contract's own framing row (permission) at the read
           k.fail("precondition", call);
           k.fail("permission", fn);
       });
    // ---- the pairing row of record item 7 between the whole-array atom of gm and a cell atom ----
    // void c2(int *q) requires(owns(gm) && owns(*q)) ensures(owns(gm) && owns(*q)) {}   int main() { int c = 0; c2(<&c | &gm[1][2] | &gm[1][3]>); return 0; }
    const auto lpair = [=](Kit& k, int mode) -> const Expr*
    {
        Global* g {gmat(k, "gm", {{"1", "2", "3"}, {"4"}})};
        Function* c2 {k.fn("c2", k.vd, {{"q", k.ptr(k.i32)}})};
        auto qa = [&](int amount)
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(c2, 0), "q"), amount, k.at());
        };
        k.req(c2, k.land(gatom(k, AtomKind::Owns, g, 0), qa(1)));
        k.ens(c2, k.land(gatom(k, AtomKind::Owns, g, 0), qa(1)));
        c2->body = k.blkc({});
        k.done(c2);
        Function* m {k.fn("main", k.i32)};
        Var* c {k.cell(m, "c", k.i32)};
        const Expr* arg {mode == 0 ? k.addr(c) : gplace(k, g, "1", "2")};
        const Expr* callx {k.call(c2, {arg})};
        m->body = k.blkc({k.decl(c, N(k, "0")), k.es(callx), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return callx;
    };
    lc("e1e2e_gmat_pair_cell_outside",
       "c2(&c) with owns(gm) && owns(*q): the cell is another object: the pairing row passes",
       [=](Kit& k)
       {
           lpair(k, 0);
       });
    lc("e1e2e_gmat_pair_cell_alias_fail",
       "c2(&gm[1][2]): the cell is one the whole-array atom holds: the pairing row fails (permission, which may be one object)",
       [=](Kit& k)
       {
           k.fail_only("permission", lpair(k, 1)->loc, 1U, 0U, "which may be one object");
       });
    rc("e1e2e_rej_gmat_static_contract_read", "its name is not in scope there",
       [=](Kit& k)
       {
           // void f(void) { static int sm[2][3]; } with requires(sm[1][0] == 0)
           Function* f {k.fn("f", k.vd)};
           Global* g {gmat(k, "sm", {}, false, f)};
           k.req(f, k.eq(gread(k, g, "1", "0"), N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    lc("e1e2e_gmat_bounds_fail",
       "gm[i][j] with i == 2 or j == 3 in main: out-of-bounds, the first index against the matrix, the second against the row's own length",
       [=](Kit& k)
       {
           Global* g {gmat(k, "gm", {})};
           Function* f {k.fn("main", k.i32)};
           Var* i {k.local(f, "i", k.i32)};
           Var* j {k.local(f, "j", k.i32)};
           const Expr* rd {
               k.deref(k.b.elem_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), k.v(i), k.at()), k.v(j), k.at()))};
           const Stmt* c {k.chk(k.eq_c(rd, k.i32, "0"))};
           f->body = k.blkc({k.decl(i, N(k, "2")), k.decl(j, N(k, "0")), c, k.ret(N(k, "0"))});
           k.done(f);
           k.prog.set_entry(f);
           k.fail_only("out-of-bounds", rd->loc, 2U, 0U);
       });
}

// The reference count of cells reached through an index step, written as the recursion of the value view's width (the formula check.cpp had before the one definition)
unsigned long long e1e2e_width(const Type& t)
{
    if (t.is_array())
    {
        return t.count * (t.element != nullptr ? e1e2e_width(*t.element) : 1U);
    }
    if (!t.is_struct() || t.struct_type == nullptr)
    {
        return 1U;
    }
    unsigned long long n {0U};
    for (const StructMember& m : t.struct_type->members)
    {
        n += e1e2e_width(m.type);
    }
    return n;
}
unsigned long long e1e2e_index_cells(const StructType& st)
{
    unsigned long long n {0U};
    for (const StructMember& m : st.members)
    {
        if (m.type.is_array())
        {
            n += m.type.count * (m.type.element != nullptr ? e1e2e_width(*m.type.element) : 1U);
        }
        else if (m.type.is_struct() && m.type.struct_type != nullptr)
        {
            n += e1e2e_index_cells(*m.type.struct_type);
        }
    }
    return n;
}

// ---- decision 0064 items 3, 7 and 13 (Part E, slice 3b): a MATRIX MEMBER of a structure (`struct U { int n; int m[2][3]; }`) ----
void define_part_e_matrix_member_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct U { int n; int m[2][3]; }
    auto U = [](Kit& k) -> StructType*
    {
        return k.strukt("U", {{"n", k.i32}, {"m", e1e2e_matrix(k)}});
    };
    // the address of u.m[i][j] for the struct object whose address is `base`
    auto place = [](Kit& k, const Expr* base, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(base, 1), i, k.at()), j, k.at());
    };
    // struct U u; u.m[1][2] = 5; check(u.m[1][2] == want);
    const auto local = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const Stmt* c {k.chk(k.eq(k.deref(place(k, k.addr(u), N(k, "1"), N(k, "2"))), N(k, want)))};
        f->body = k.blkc({k.decl(u, nullptr), k.store(place(k, k.addr(u), N(k, "1"), N(k, "2")), N(k, "5")), c});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_mmember_local_rw", "a local structure with a matrix member: u.m[1][2] written and read back",
       [=](Kit& k)
       {
           local(k, "5");
       });
    lc("e1e2e_mmember_local_wrong_fail", "the same: the check that u.m[1][2] is 6 fails",
       [=](Kit& k)
       {
           k.fail("check", local(k, "6"));
       });
    lc("e1e2e_mmember_local_vacuity_fail",
       "the allocation's facts are satisfiable: a check(false) after the write is reached and fails",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", S)};
           const Stmt* c {k.chk(k.no())};
           f->body = k.blkc({k.decl(u, nullptr), k.store(place(k, k.addr(u), N(k, "1"), N(k, "2")), N(k, "5")), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    // struct U u; u.n = 1; zero fill of u.m; check(u.m[1][2] == want);   (the encoder's order: the zero fill of the member, then the stores)
    const auto zero = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const Stmt* c {k.chk(k.eq(k.deref(place(k, k.addr(u), N(k, "1"), N(k, "2"))), N(k, want)))};
        f->body = k.blkc(
            {k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), k.lstore(k.addr(u), 0, N(k, "1")), c});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_mmember_zero_fill", "the zero fill of a matrix member: every cell of every row is zero and set",
       [=](Kit& k)
       {
           zero(k, "0");
       });
    lc("e1e2e_mmember_zero_fill_wrong_fail", "the zero fill: the check that u.m[1][2] is 1 fails",
       [=](Kit& k)
       {
           k.fail("check", zero(k, "1"));
       });
    lc("e1e2e_mmember_zero_fill_vacuity_fail",
       "the zero fill's contract is satisfiable: a check(false) after it is reached and fails",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", S)};
           const Stmt* c {k.chk(k.no())};
           f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    lc("e1e2e_mmember_uninitialized_read_fail",
       "u.m[1][2] read before any write: uninitialized (the allocation's flags are false)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", S)};
           const Expr* r {k.deref(place(k, k.addr(u), N(k, "1"), N(k, "2")))};
           f->body = k.blkc({k.decl(u, nullptr), k.store(place(k, k.addr(u), N(k, "1"), N(k, "1")), N(k, "5")),
                             k.chk(k.eq(r, N(k, "0")))});
           k.done(f);
           k.fail("uninitialized", r->loc);
       });
    // int f(int i, int j) requires(0 <= i <= hi_i && 0 <= j <= hi_j) { struct U u; u.m[i][j] = 3; }
    const auto sym = [=](Kit& k, const char* hi_i, const char* hi_j) -> const Stmt*
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", hi_i), k.range(k.F(f, 1), k.i32, "0", hi_j)));
        Var* u {k.scell(f, "u", S)};
        const Stmt* w {k.store(place(k, k.addr(u), k.pv(f, 0), k.pv(f, 1)), N(k, "3"))};
        f->body = k.blkc({k.decl(u, nullptr), w});
        k.done(f);
        return w;
    };
    lc("e1e2e_mmember_symbolic_index", "u.m[i][j] written for symbolic indexes in range: both bound rows hold",
       [=](Kit& k)
       {
           sym(k, "1", "2");
       });
    lc("e1e2e_mmember_first_index_high_fail",
       "u.m[i][j] with i == 2: out-of-bounds at the store, the first index, before the cell's rows",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", sym(k, "2", "2")->loc, 2U, 0U);
       });
    lc("e1e2e_mmember_second_index_high_fail",
       "u.m[i][j] with j == 3: out-of-bounds at the store, the second index against the row's own length",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", sym(k, "1", "3")->loc, 2U, 1U);
       });
    // void keep(int *p, int n) requires(owns_range(p, n)); void f(void) { struct U u; u.m[1][0] = 0; keep(&u.m[1][0], 3); }: the callee keeps the row's cells
    lc("e1e2e_mmember_row_kept_fail",
       "the row u.m[1] given to a callee that keeps its cells: lifetime at the scope exit",
       [=](Kit& k)
       {
           Function* g {k.fn("keep", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.trusted(g);
           StructType* S {U(k)};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", S)};
           const Stmt* bs {k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                                   k.es(k.call(g, {place(k, k.addr(u), N(k, "1"), N(k, "0")), N(k, "3")}))})};
           f->body = bs;
           k.done(f);
           // the four rows of the scope exit are u.n's, the member's value permission, its flag permission and the liveness; the second fails (the callee took the row's value permissions) and ends the path
           k.fail_only("lifetime", bs->end_loc, 4U, 1U);
       });
    // ---- 3b-2: the member under owns(*p) and owns(p->m): atoms, calls and returns ----
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto PU = [](Kit& k, StructType* S)
    {
        return k.ptr(k.st(S));
    };
    // void put(struct U *p) requires(<owns(p->m) | owns(*p) | reads(*p)>) ensures(<owns(p->m) | owns(*p) | none>) { p->m[1][2] = 5; [p->n = 1;] }
    // `shape`: 0 owns(p->m), 1 owns(*p), 2 reads(*p) in the requires (the writes fail), 3 owns(p->m) in requires and nothing in ensures
    auto make_put = [=](Kit& k, StructType* S, int shape, bool writes_n) -> Function*
    {
        Function* f {k.fn("put", k.vd, {{"p", PU(k, S)}})};
        const Expr* atom_m {k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 1), 0, k.at())};
        const Expr* atom_s {k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at())};
        const Expr* atom_r {k.b.atom(AtomKind::Reads, FP(k, f, 0), 0, k.at())};
        k.req(f, shape == 0 || shape == 3 ? atom_m : shape == 1 ? atom_s : atom_r);
        if (shape == 0)
        {
            k.ens(f, k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 1), 0, k.at()));
        }
        else if (shape == 1)
        {
            k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
        }
        std::vector<const Stmt*> body {k.store(
            k.b.elem_addr(k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "1"), k.at()), N(k, "2"), k.at()), N(k, "5"))};
        if (writes_n)
        {
            body.push_back(k.lstore(k.pv(f, 0), 0, N(k, "1")));
        }
        f->body = k.blk(body);
        k.done(f);
        return f;
    };
    // struct U u; u.n = 7; zero fill of u.m; put(&u); u.m[0][0] = 1; check(u.n == 7)
    auto use_put = [=](Kit& k, StructType* S, Function* put, const char* want_n) -> const Stmt*
    {
        Function* f {k.fn("use", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const Stmt* c {want_n != nullptr ? k.chk(k.eq(k.deref(k.mem(k.addr(u), 0)), N(k, want_n))) : k.chk(k.yes())};
        f->body =
            k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), k.lstore(k.addr(u), 0, N(k, "7")),
                    k.es(k.call(put, {k.addr(u)})), k.store(place(k, k.addr(u), N(k, "0"), N(k, "0")), N(k, "1")), c});
        k.done(f);
        return c;
    };
    lc("e1e2e_mmember_owns_member_call",
       "owns(p->m) in requires and ensures: the callee writes p->m[1][2]; the caller keeps u.n and writes u.m[0][0] after the call",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 0, false)};
           use_put(k, S, put, "7");
       });
    lc("e1e2e_mmember_owns_member_call_wrong_fail", "the same: check(u.n == 8) fails (the callee never touched u.n)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 0, false)};
           k.fail("check", use_put(k, S, put, "8")->loc);
       });
    lc("e1e2e_mmember_owns_whole_call",
       "owns(*p) in requires and ensures: the callee writes p->m[1][2] and p->n; the caller writes u.m[0][0] after the call",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 1, true)};
           use_put(k, S, put, nullptr);
       });
    lc("e1e2e_mmember_owns_whole_call_value_unknown_fail",
       "owns(*p) promises no value: check(u.n == 7) after the call fails (the callee may have written u.n)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 1, true)};
           k.fail("check", use_put(k, S, put, "7")->loc);
       });
    lc("e1e2e_mmember_owns_member_writes_cell_fail", "p->n written under owns(p->m): permission at the store",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 0, true)};
           k.fail("permission", put->body->stmts.back()->loc);
       });
    lc("e1e2e_mmember_reads_write_fail", "p->m[1][2] written under reads(*p): permission at the store",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 2, false)};
           k.fail("permission", put->body->stmts.back()->loc);
       });
    lc("e1e2e_mmember_owns_member_not_returned_fail",
       "owns(p->m) in requires and nothing in ensures: the caller's write of u.m[0][0] after the call fails (permission)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* put {make_put(k, S, 3, false)};
           Function* f {k.fn("use", k.vd)};
           Var* u {k.scell(f, "u", S)};
           const Stmt* w {k.store(place(k, k.addr(u), N(k, "0"), N(k, "0")), N(k, "1"))};
           f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                             k.lstore(k.addr(u), 0, N(k, "7")), k.es(k.call(put, {k.addr(u)})), w});
           k.done(f);
           k.fail("permission", w->loc);
       });
    // ---- 3b-2: the pairing of the atoms (record item 7, by slots) ----
    // void two(struct U *p, struct U *q) requires(owns(*p) && owns(*q)) ensures(owns(*p) && owns(*q)) {}
    const auto two_structs = [=](Kit& k, StructType* S) -> Function*
    {
        Function* g {k.fn("two", k.vd, {{"p", PU(k, S)}, {"q", PU(k, S)}})};
        const Expr* both {
            k.land(k.b.atom(AtomKind::Owns, FP(k, g, 0), 0, k.at()), k.b.atom(AtomKind::Owns, FP(k, g, 1), 1, k.at()))};
        k.req(g, both);
        k.ens(g, k.land(k.b.atom(AtomKind::Owns, FP(k, g, 0), 0, k.at()),
                        k.b.atom(AtomKind::Owns, FP(k, g, 1), 1, k.at())));
        k.trusted(g);
        return g;
    };
    // void f(void) { struct U u; struct U v; zero fills; two(&u, <&u | &v>); }
    const auto call_two = [=](Kit& k, bool same) -> const Expr*
    {
        StructType* S {U(k)};
        Function* g {two_structs(k, S)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* v {k.scell(f, "v", S)};
        const Expr* callx {k.call(g, {k.addr(u), same ? k.addr(u) : k.addr(v)})};
        f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                          k.lstore(k.addr(u), 0, N(k, "7")), k.decl(v, nullptr),
                          k.b.array_zero(k.mem(k.addr(v), 1), k.at()), k.lstore(k.addr(v), 0, N(k, "7")), k.es(callx)});
        k.done(f);
        return callx;
    };
    lc("e1e2e_mmember_pair_two_structures_disjoint",
       "two(&u, &v) with owns(*p) && owns(*q) over two structures with a matrix member: no overlap",
       [=](Kit& k)
       {
           call_two(k, false);
       });
    lc("e1e2e_mmember_pair_two_structures_alias_fail",
       "two(&u, &u): the two atoms' matrix members share every slot (permission, at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", call_two(k, true)->loc, 32U, 14U);
       });
    // void mix(struct U *p, int *q) requires(owns(p->m) && owns(*q)) ensures(owns(p->m) && owns(*q)) {}
    const auto mix = [=](Kit& k, StructType* S) -> Function*
    {
        Function* g {k.fn("mix", k.vd, {{"p", PU(k, S)}, {"q", k.ptr(k.i32)}})};
        const Expr* both {k.land(k.b.atom(AtomKind::Owns, k.mem(FP(k, g, 0), 1), 0, k.at()),
                                 k.b.atom(AtomKind::Owns, FP(k, g, 1), 1, k.at()))};
        k.req(g, both);
        k.ens(g, k.land(k.b.atom(AtomKind::Owns, k.mem(FP(k, g, 0), 1), 0, k.at()),
                        k.b.atom(AtomKind::Owns, FP(k, g, 1), 1, k.at())));
        k.trusted(g);
        return g;
    };
    // void f(void) { struct U u; zero fill; mix(&u, <&u.n | &u.m[1][2]>); }
    const auto call_mix = [=](Kit& k, bool inside) -> const Expr*
    {
        StructType* S {U(k)};
        Function* g {mix(k, S)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const Expr* callx {
            k.call(g, {k.addr(u), inside ? place(k, k.addr(u), N(k, "1"), N(k, "2")) : k.mem(k.addr(u), 0)})};
        f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                          k.lstore(k.addr(u), 0, N(k, "7")), k.es(callx)});
        k.done(f);
        return callx;
    };
    lc("e1e2e_mmember_pair_member_and_cell_disjoint",
       "mix(&u, &u.n) with owns(p->m) && owns(*q): the cell u.n is outside the matrix member",
       [=](Kit& k)
       {
           call_mix(k, false);
       });
    lc("e1e2e_mmember_pair_member_and_cell_alias_fail",
       "mix(&u, &u.m[1][2]) with owns(p->m) && owns(*q): the cell is a cell of the matrix member (permission, at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", call_mix(k, true)->loc, 22U, 11U);
       });
    // ---- 3b-2: sequencing (record item 11): a call that holds the structure beside an access or another call in one full expression ----
    // int put(struct U *p) requires(owns(p->m)) ensures(owns(p->m)) { return 1; }   void f(void) { struct U u; zero fill; u.n = 7; int w = 1; int z = put(&u) + <rhs>; }
    const auto mseq = [=](Kit& k, int mode) -> const Expr*
    {
        StructType* S {U(k)};
        Function* put {k.fn("put", k.i32, {{"p", PU(k, S)}})};
        k.req(put, k.b.atom(AtomKind::Owns, k.mem(FP(k, put, 0), 1), 0, k.at()));
        k.ens(put, k.land(k.b.atom(AtomKind::Owns, k.mem(FP(k, put, 0), 1), 0, k.at()), k.eq(k.R(put), N(k, "1"))));
        put->body = k.blk({k.ret(N(k, "1"))});
        k.done(put);
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* w {k.local(f, "w", k.i32)};
        Var* z {k.local(f, "z", k.i32)};
        const Expr* callx {k.call(put, {k.addr(u)})};
        // mode 0: beside a cell of u.m (overlap), 1: beside u.n (disjoint: the structure's other member), 2: beside a second call (overlap)
        const Expr* rhs {mode == 0   ? k.deref(place(k, k.addr(u), N(k, "1"), N(k, "2")))
                         : mode == 1 ? k.deref(k.mem(k.addr(u), 0))
                                     : k.call(put, {k.addr(u)})};
        f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                          k.lstore(k.addr(u), 0, N(k, "7")), k.decl(w, N(k, "1")), k.decl(z, k.add(callx, rhs))});
        k.done(f);
        return rhs;
    };
    lc("e1e2e_mmember_seq_disjoint", "put(&u) + u.n: the call holds u.m, u.n is another object: no row fails",
       [=](Kit& k)
       {
           mseq(k, 1);
       });
    lc("e1e2e_mmember_seq_call_and_cell_overlap_fail",
       "put(&u) + u.m[1][2]: the call holds every cell of u.m and the read names one of them (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", mseq(k, 0)->loc);
       });
    lc("e1e2e_mmember_seq_two_calls_overlap_fail", "put(&u) + put(&u): two calls that both hold u.m (unsequenced)",
       [=](Kit& k)
       {
           k.fail("unsequenced", mseq(k, 2)->loc);
       });
    // ---- 3b-3: a GLOBAL structure with a matrix member (decision 0064 item 9): program_start's row group of the member, the length facts, main's reads and writes, a call that owns it, a loop in main ----
    // struct U gu = {7, {{1, 2, 3}, {4}}};   (`unknown`: the linkage and escape case, every cell unknown)
    const auto gu_global = [=](Kit& k, StructType* S, bool unknown) -> Global*
    {
        Global* g {k.prog.add_global("gu", k.st(S), k.at(), nullptr)};
        GlobalLeaf ln;
        ln.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
        ln.init = k.num(k.i32, "7");
        ln.value = "7";
        ln.loc = k.at();
        g->leaves.push_back(ln);
        GlobalLeaf lm;
        lm.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
        lm.rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
        lm.loc = k.at();
        if (!unknown)
        {
            for (const std::vector<const char*>& r : std::vector<std::vector<const char*>> {{"1", "2", "3"}, {"4"}})
            {
                GlobalLeaf row;
                row.kind = GlobalInit::Value;
                row.rest = ArrayRest::Zero;
                row.loc = k.at();
                for (const char* v : r)
                {
                    GlobalLeaf c;
                    c.kind = GlobalInit::Value;
                    c.value = v;
                    c.init = k.num(k.i32, v);
                    c.loc = k.at();
                    row.elems.push_back(c);
                }
                lm.elems.push_back(row);
            }
        }
        g->leaves.push_back(lm);
        return g;
    };
    auto gucell = [=](Kit& k, const Global* g, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), N(k, i), k.at()), N(k, j), k.at());
    };
    // int main(void) { check(gu.n == 7 && gu.m[0][1] == 2 && gu.m[1][0] == 4 && gu.m[1][2] == <want>); [gu.m[1][2] = 5; check(gu.m[1][2] == 5 && gu.m[0][0] == 1);] return 0; }
    const auto gu_main = [=](Kit& k, const char* want, bool write) -> const Stmt*
    {
        StructType* S {U(k)};
        Global* g {gu_global(k, S, false)};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c1 {k.chk(k.land(k.land(k.eq(k.deref(k.mem(k.b.global_addr(g, k.at()), 0)), N(k, "7")),
                                            k.eq(k.deref(gucell(k, g, "0", "1")), N(k, "2"))),
                                     k.land(k.eq(k.deref(gucell(k, g, "1", "0")), N(k, "4")),
                                            k.eq(k.deref(gucell(k, g, "1", "2")), N(k, want)))))};
        std::vector<const Stmt*> body {c1};
        if (write)
        {
            body.push_back(k.store(gucell(k, g, "1", "2"), N(k, "5")));
            body.push_back(k.chk(k.land(k.eq(k.deref(gucell(k, g, "1", "2")), N(k, "5")),
                                        k.eq(k.deref(gucell(k, g, "0", "0")), N(k, "1")))));
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return c1;
    };
    lc("e1e2e_gumatrix_main_read",
       "struct U gu = {7, {{1, 2, 3}, {4}}}: main reads every listed cell and the zero rest of the member",
       [=](Kit& k)
       {
           gu_main(k, "0", false);
       });
    lc("e1e2e_gumatrix_main_read_wrong_fail", "the same: the check that gu.m[1][2] is 1 fails (the zero rest)",
       [=](Kit& k)
       {
           k.fail("check", gu_main(k, "1", false)->loc);
       });
    lc("e1e2e_gumatrix_main_write",
       "gu.m[1][2] = 5 in main: the member's cells are held with write permission from program_start, the other cells keep their values",
       [=](Kit& k)
       {
           gu_main(k, "0", true);
       });
    lc("e1e2e_gumatrix_main_vacuity_fail",
       "program_start's facts for the member are satisfiable: a check(false) after the reads is reached and fails",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Global* g {gu_global(k, S, false)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.no())};
           m->body = k.blkc({k.chk(k.eq(k.deref(gucell(k, g, "0", "1")), N(k, "2"))), c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2e_gumatrix_unknown_cells_fail",
       "an unknown initial value (linkage): gu.m[0][0] == 1 is not provable (check)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Global* g {gu_global(k, S, true)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq(k.deref(gucell(k, g, "0", "0")), N(k, "1")))};
           m->body = k.blkc({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    // int main(void) { put(&gu); gu.m[0][0] = 9; [check(gu.m[1][0] == 4)]; return 0; }   put: shape 0 owns(p->m) (the member only), 1 owns(*p)
    const auto gu_call = [=](Kit& k, int shape, const char* want) -> const Stmt*
    {
        StructType* S {U(k)};
        Global* g {gu_global(k, S, false)};
        Function* put {make_put(k, S, shape, shape == 1)};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c {want != nullptr ? k.chk(k.eq(k.deref(gucell(k, g, "1", "0")), N(k, want))) : k.chk(k.yes())};
        m->body = k.blkc({k.es(k.call(put, {k.b.global_addr(g, k.at())})), k.store(gucell(k, g, "0", "0"), N(k, "9")),
                          c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_gumatrix_call_owns_member",
       "main: put(&gu) with owns(p->m): the callee writes gu.m[1][2] and hands the member back; the cell gu.m[1][0] is not the callee's to change only if the callee ensured it, so main writes gu.m[0][0] after the call",
       [=](Kit& k)
       {
           gu_call(k, 0, nullptr);
       });
    lc("e1e2e_gumatrix_call_owns_member_wrong_fail",
       "the same: nothing says gu.m[1][0] is still 4 (the callee owns the whole member and promises no values): the check fails",
       [=](Kit& k)
       {
           k.fail("check", gu_call(k, 0, "4")->loc);
       });
    lc("e1e2e_gumatrix_call_owns_whole",
       "main: put(&gu) with owns(*p): the callee writes the member and u.n; main writes gu.m[0][0] after the call",
       [=](Kit& k)
       {
           gu_call(k, 1, nullptr);
       });
    // int main(void) { int i = 0; while (i < 3) inv 0 <= i <= 3 && gu.m[0][1] == 2 [&& gu.m[1][i] ...] decreases 3 - i { [gu.m[1][i] = 1;] i++; } return 0; }
    const auto gu_loop = [=](Kit& k, bool write) -> const Clause
    {
        StructType* S {U(k)};
        Global* g {gu_global(k, S, false)};
        Function* m {k.fn("main", k.i32)};
        Var* i {k.local(m, "i", k.i32)};
        const Clause keep {k.cl(k.eq(k.deref(gucell(k, g, "0", "1")), N(k, "2")))};
        std::vector<const Stmt*> ib;
        if (write)
        {
            ib.push_back(k.store(
                k.b.elem_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), N(k, "0"), k.at()), k.v(i), k.at()),
                N(k, "1")));
        }
        ib.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")),
                                {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(ib))};
        m->body = k.blkc({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return keep;
    };
    lc("e1e2e_gumatrix_loop_main_reads",
       "a loop in main whose invariant reads gu.m[0][1] == 2: the member is an array place of the loop (cells over two bound variables), the value is kept",
       [=](Kit& k)
       {
           gu_loop(k, false);
       });
    lc("e1e2e_gumatrix_loop_main_write_loses_values_fail",
       "the loop writes gu.m[0][i]: a store by name reaches the member's place, so gu.m[0][1] == 2 is not kept (the invariant fails at the back edge)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", gu_loop(k, true).loc);
       });
    // ---- 3b-2: loops in a function that holds the structure (the member is a leaf of the held structure's range place) ----
    // void f(struct U *p) requires(<owns | reads>(*p)) ensures(owns(*p)) { int i = 0; while (i < 3) inv 0 <= i <= 3 [&& p->m[1][2] == p->m[1][2]] decreases 3 - i { p->m[1][i] = 1; i++; } }
    const auto held_loop = [=](Kit& k, bool owns, bool read_inv) -> const Stmt*
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", k.vd, {{"p", PU(k, S)}})};
        k.req(f, k.b.atom(owns ? AtomKind::Owns : AtomKind::Reads, FP(k, f, 0), 0, k.at()));
        if (owns)
        {
            k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
        }
        Var* i {k.local(f, "i", k.i32)};
        std::vector<Clause> inv {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3"))))};
        const auto pcell = [&](const Expr* idx)
        {
            return k.b.elem_addr(k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "1"), k.at()), idx, k.at());
        };
        if (read_inv)
        {
            inv.push_back(k.cl(k.eq(k.deref(pcell(N(k, "2"))), k.deref(pcell(N(k, "2"))))));
        }
        const Stmt* w {k.store(pcell(k.v(i)), N(k, "1"))};
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")), inv, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                k.blkc({w, k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        f->body = k.blkc({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return w;
    };
    lc("e1e2e_mmember_held_loop_write",
       "a loop writing p->m[1][i] under owns(*p): the member's cells are held by the loop's range place",
       [=](Kit& k)
       {
           held_loop(k, true, false);
       });
    lc("e1e2e_mmember_held_loop_write_reads_fail",
       "the same under reads(*p): the write has no write permission (permission, at the store)",
       [=](Kit& k)
       {
           k.fail("permission", held_loop(k, false, false)->loc);
       });
    lc("e1e2e_mmember_held_loop_invariant_read",
       "the loop's invariant reads p->m[1][2] under owns(*p): framed by the range place's leaf",
       [=](Kit& k)
       {
           held_loop(k, true, true);
       });
    // ---- 3b-2: what a callee may assume of two atoms the caller proved distinct (the pairing facts of its `requires`, by slots) ----
    // void mixb(struct U *p, int *q) requires(<owns(p->m) && owns(*q) | owns(*q) && owns(p->m)>) ensures(the same && *q == old(*q)) { p->m[1][2] = 1; }
    const auto mixbody = [=](Kit& k, bool cell_first, int wrong) -> SrcLoc
    {
        StructType* S {U(k)};
        Function* g {k.fn("mixb", k.vd, {{"p", PU(k, S)}, {"q", k.ptr(k.i32)}})};
        auto atoms = [&]()
        {
            const Expr* am {k.b.atom(AtomKind::Owns, k.mem(FP(k, g, 0), 1), cell_first ? 1 : 0, k.at())};
            const Expr* aq {k.b.atom(AtomKind::Owns, FP(k, g, 1), cell_first ? 0 : 1, k.at())};
            return cell_first ? k.land(aq, am) : k.land(am, aq);
        };
        k.req(g, atoms());
        const Expr* qv {k.deref(FP(k, g, 1))};
        // wrong 0: *q == old(*q) (the cell is not the matrix's); wrong 1: *q == 1 (the write may have been to *q)
        const Expr* post {wrong == 0 ? k.eq(qv, k.old(k.deref(FP(k, g, 1)))) : k.eq(qv, N(k, "1"))};
        k.ens(g, k.land(atoms(), post));
        const Stmt* w {k.store(place(k, k.pv(g, 0), N(k, "1"), N(k, "2")), N(k, "1"))};
        const Stmt* r {k.b.return_stmt(nullptr, k.at())};
        g->body = k.blkc({w, r});
        k.done(g);
        return r->loc;
    };
    lc("e1e2e_mmember_pair_assumed_member_first",
       "the callee writes p->m[1][2] and keeps *q: it may assume the cell *q is no cell of the matrix (owns(p->m) && owns(*q))",
       [=](Kit& k)
       {
           mixbody(k, false, 0);
       });
    lc("e1e2e_mmember_pair_assumed_cell_first", "the same with the atoms in the other order (owns(*q) && owns(p->m))",
       [=](Kit& k)
       {
           mixbody(k, true, 0);
       });
    lc("e1e2e_mmember_pair_assumed_wrong_fail",
       "the callee writes p->m[1][2] = 1 and ensures *q == 1: the postcondition does not follow (postcondition, at the return)",
       [=](Kit& k)
       {
           k.fail("postcondition", mixbody(k, false, 1));
       });
    // ---- the WHOLE matrix given to a row parameter (record 0064 item 13, position (iii)): a global matrix `zz(gm, 2)` and a matrix member `zz(u.m, 2)` ----
    auto rowptr = [](Kit& k)
    {
        return k.ptr(e1e2e_row(k));
    };
    auto RA = [](Kit& k, Function* f, AtomKind kind, int amount)
    {
        return k.b.range_atom(kind, k.b.text(k.F(f, 0), "a"), k.b.text(k.F(f, 1), "n"), amount, k.at());
    };
    // void zz(int (*a)[3], int n) requires(<owns_range | reads_range>(a, n)) ensures(<the same | nothing>);  `mode`: 0 reads and reads, 1 owns and owns, 2 owns and nothing
    auto zz = [=](Kit& k, int mode) -> Function*
    {
        Function* g {k.fn("zz", k.vd, {{"a", rowptr(k)}, {"n", k.i32}})};
        const AtomKind kind {mode == 0 ? AtomKind::ReadsRange : AtomKind::OwnsRange};
        k.req(g, RA(k, g, kind, 0));
        if (mode != 2)
        {
            k.ens(g, RA(k, g, kind, 0));
        }
        k.trusted(g);
        return g;
    };
    // void pairf(int (*a)[3], int n, int *q) requires(owns_range(a, n) && owns(*q)) ensures(owns_range(a, n) && owns(*q))
    auto pairf = [=](Kit& k) -> Function*
    {
        Function* g {k.fn("pairf", k.vd, {{"a", rowptr(k)}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
        const Expr* both {
            k.land(RA(k, g, AtomKind::OwnsRange, 0), k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 2), "q"), 1, k.at()))};
        k.req(g, both);
        k.ens(g,
              k.land(RA(k, g, AtomKind::OwnsRange, 0), k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 2), "q"), 1, k.at())));
        k.trusted(g);
        return g;
    };
    // int gm[2][3] = {0};  (the zero global)
    auto zglobal = [=](Kit& k) -> Global*
    {
        Global* g {k.prog.add_global("gm", e1e2e_matrix(k), k.at(), nullptr)};
        g->rest = ArrayRest::Zero;
        return g;
    };
    auto gcell = [=](Kit& k, Global* g, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, i), k.at()), N(k, j), k.at());
    };
    // int main(void) { zz(gm, <count>); [gm[0][0] = 1;] check(gm[1][2] == <want>); return 0; }
    struct Called
    {
        const Expr* call {nullptr};
        const Stmt* write {nullptr};
        const Stmt* check {nullptr};
    };
    // `want` null: no value check (an owns callee with no ensures-value promises none)
    const auto gcall = [=](Kit& k, int mode, const char* count, bool write, const char* want) -> Called
    {
        Global* g {zglobal(k)};
        Function* zf {zz(k, mode)};
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(zf, {k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "0"), k.at()), N(k, count)})};
        const Stmt* w {write ? k.store(gcell(k, g, "0", "0"), N(k, "1")) : k.es(k.yes())};
        const Stmt* c {want != nullptr ? k.chk(k.eq(k.deref(gcell(k, g, "1", "2")), N(k, want))) : k.chk(k.yes())};
        m->body = k.blkc({k.es(callx), w, c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return {callx, w, c};
    };
    lc("e1e2e_gmat_to_row_param_reads",
       "main: zz(gm, 2) with reads_range: both rows are borrowed and returned, gm[1][2] is still 0",
       [=](Kit& k)
       {
           gcall(k, 0, "2", false, "0");
       });
    lc("e1e2e_gmat_to_row_param_reads_wrong_fail", "the same checking gm[1][2] == 1: the check fails",
       [=](Kit& k)
       {
           k.fail("check", gcall(k, 0, "2", false, "1").check->loc);
       });
    lc("e1e2e_gmat_to_row_param_owns",
       "main: zz(gm, 2) with owns_range and ensures: gm[0][0] is writable after the call",
       [=](Kit& k)
       {
           gcall(k, 1, "2", true, nullptr);
       });
    lc("e1e2e_gmat_to_row_param_too_many_rows_fail",
       "zz(gm, 3) over a matrix of 2 rows: the shape row at the call (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", gcall(k, 0, "3", false, "0").call->loc, 10U, 0U);
       });
    lc("e1e2e_gmat_to_row_param_owns_not_returned_fail",
       "zz(gm, 2) with owns_range and no ensures: the write of gm[0][0] after the call fails (permission)",
       [=](Kit& k)
       {
           k.fail("permission", gcall(k, 2, "2", true, nullptr).write->loc);
       });
    // int main(void) { pairf(gm, <rows>, &gm[1][2]); return 0; }
    const auto gpair = [=](Kit& k, const char* rows) -> const Expr*
    {
        Global* g {zglobal(k)};
        Function* pf {pairf(k)};
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(
            pf, {k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "0"), k.at()), N(k, rows), gcell(k, g, "1", "2")})};
        m->body = k.blkc({k.es(callx), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return callx;
    };
    lc("e1e2e_gmat_pair_disjoint_cell",
       "pairf(gm, 1, &gm[1][2]): the range holds row 0 only, the cell is in row 1: no overlap",
       [=](Kit& k)
       {
           gpair(k, "1");
       });
    lc("e1e2e_gmat_pair_overlapping_cell_fail",
       "pairf(gm, 2, &gm[1][2]): the cell is inside the range: the call's atoms overlap (permission, once at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", gpair(k, "2")->loc, 20U, 9U);
       });
    // struct U u; zero fill of u.m; zz(u.m, <count>); [u.m[0][0] = 1;] check(u.m[1][2] == <want>)
    const auto mcall = [=](Kit& k, int mode, const char* count, bool write, const char* want) -> Called
    {
        StructType* S {U(k)};
        Function* zf {zz(k, mode)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const Expr* callx {k.call(zf, {k.b.elem_addr(k.mem(k.addr(u), 1), N(k, "0"), k.at()), N(k, count)})};
        const Stmt* w {write ? k.store(place(k, k.addr(u), N(k, "0"), N(k, "0")), N(k, "1")) : k.es(k.yes())};
        const Stmt* c {want != nullptr ? k.chk(k.eq(k.deref(place(k, k.addr(u), N(k, "1"), N(k, "2"))), N(k, want)))
                                       : k.chk(k.yes())};
        f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                          k.lstore(k.addr(u), 0, N(k, "7")), k.es(callx), w, c});
        k.done(f);
        return {callx, w, c};
    };
    lc("e1e2e_mmember_to_row_param_reads",
       "zz(u.m, 2) with reads_range: both rows of the member are borrowed and returned, u.m[1][2] is still 0",
       [=](Kit& k)
       {
           mcall(k, 0, "2", false, "0");
       });
    lc("e1e2e_mmember_to_row_param_reads_wrong_fail", "the same checking u.m[1][2] == 1: the check fails",
       [=](Kit& k)
       {
           k.fail("check", mcall(k, 0, "2", false, "1").check->loc);
       });
    lc("e1e2e_mmember_to_row_param_owns",
       "zz(u.m, 2) with owns_range and ensures: u.m[0][0] is writable after the call",
       [=](Kit& k)
       {
           mcall(k, 1, "2", true, nullptr);
       });
    lc("e1e2e_mmember_to_row_param_too_many_rows_fail",
       "zz(u.m, 3) over a member of 2 rows: the shape row at the call (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", mcall(k, 0, "3", false, "0").call->loc, 10U, 0U);
       });
    lc("e1e2e_mmember_to_row_param_owns_not_returned_fail",
       "zz(u.m, 2) with owns_range and no ensures: the write of u.m[0][0] after the call fails (permission)",
       [=](Kit& k)
       {
           k.fail("permission", mcall(k, 2, "2", true, nullptr).write->loc);
       });
    // void f(void) { struct U u; zero fill; pairf(u.m, <rows>, &u.m[1][2]); }
    const auto mpair = [=](Kit& k, const char* rows) -> const Expr*
    {
        StructType* S {U(k)};
        Function* pf {pairf(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const Expr* callx {k.call(pf, {k.b.elem_addr(k.mem(k.addr(u), 1), N(k, "0"), k.at()), N(k, rows),
                                       place(k, k.addr(u), N(k, "1"), N(k, "2"))})};
        f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), k.es(callx)});
        k.done(f);
        return callx;
    };
    lc("e1e2e_mmember_pair_disjoint_cell",
       "pairf(u.m, 1, &u.m[1][2]): the range holds row 0 only, the cell is in row 1: no overlap",
       [=](Kit& k)
       {
           mpair(k, "1");
       });
    lc("e1e2e_mmember_pair_overlapping_cell_fail",
       "pairf(u.m, 2, &u.m[1][2]): the cell is inside the range: the call's atoms overlap (permission, once at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", mpair(k, "2")->loc, 20U, 9U);
       });
    // the whole member matrix of a GLOBAL structure given to a row parameter: main() { zz(gu.m, <count>); check(gu.m[1][2] == <want>); }   (zz and pairf are defined with the whole-matrix cases below)
    const auto gu_row_call = [=](Kit& k, int mode, const char* count, const char* want) -> Called
    {
        StructType* S {U(k)};
        Global* g {gu_global(k, S, false)};
        Function* zf {zz(k, mode)};
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {
            k.call(zf, {k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), N(k, "0"), k.at()), N(k, count)})};
        const Stmt* c {want != nullptr ? k.chk(k.eq(k.deref(gucell(k, g, "1", "2")), N(k, want))) : k.chk(k.yes())};
        m->body = k.blkc({k.es(callx), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return {callx, nullptr, c};
    };
    lc("e1e2e_gumatrix_to_row_param_reads",
       "main: zz(gu.m, 2) with reads_range: the member's rows are borrowed and returned, gu.m[1][2] is still 0",
       [=](Kit& k)
       {
           gu_row_call(k, 0, "2", "0");
       });
    lc("e1e2e_gumatrix_to_row_param_reads_wrong_fail", "the same checking gu.m[1][2] == 1: the check fails",
       [=](Kit& k)
       {
           k.fail("check", gu_row_call(k, 0, "2", "1").check->loc);
       });
    lc("e1e2e_gumatrix_to_row_param_owns", "main: zz(gu.m, 2) with owns_range and ensures",
       [=](Kit& k)
       {
           gu_row_call(k, 1, "2", nullptr);
       });
    lc("e1e2e_gumatrix_to_row_param_too_many_rows_fail",
       "zz(gu.m, 3) over a member of 2 rows: the shape row at the call (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", gu_row_call(k, 0, "3", "0").call->loc, 10U, 0U);
       });
    // int main(void) { pairf(gu.m, <rows>, &gu.m[1][2]); return 0; }
    const auto gu_pair = [=](Kit& k, const char* rows) -> const Expr*
    {
        StructType* S {U(k)};
        Global* g {gu_global(k, S, false)};
        Function* pf {pairf(k)};
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(pf, {k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), N(k, "0"), k.at()),
                                       N(k, rows), gucell(k, g, "1", "2")})};
        m->body = k.blkc({k.es(callx), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return callx;
    };
    lc("e1e2e_gumatrix_pair_disjoint_cell",
       "pairf(gu.m, 1, &gu.m[1][2]): the range holds row 0 only, the cell is in row 1",
       [=](Kit& k)
       {
           gu_pair(k, "1");
       });
    lc("e1e2e_gumatrix_pair_overlapping_cell_fail",
       "pairf(gu.m, 2, &gu.m[1][2]): the cell is inside the range (permission, once at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", gu_pair(k, "2")->loc, 20U, 9U);
       });
    // void f(struct U *p) requires(<owns | reads>(*p)) ensures(<owns | reads>(*p)) { zz(p->m, <count>); }   (a function that holds the structure through its atom gives the member's rows on)
    const auto p_row_call = [=](Kit& k, bool owns, int mode, const char* count,
                                SrcLoc* ret_loc = nullptr) -> const Expr*
    {
        StructType* S {U(k)};
        Function* zf {zz(k, mode)};
        Function* f {k.fn("f", k.vd, {{"p", PU(k, S)}})};
        k.req(f, k.b.atom(owns ? AtomKind::Owns : AtomKind::Reads, FP(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(owns ? AtomKind::Owns : AtomKind::Reads, FP(k, f, 0), 0, k.at()));
        const Expr* callx {k.call(zf, {k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "0"), k.at()), N(k, count)})};
        const Stmt* r {k.b.return_stmt(nullptr, k.at())};
        f->body = k.blkc({k.es(callx), r});
        k.done(f);
        if (ret_loc != nullptr)
        {
            *ret_loc = r->loc;
        }
        return callx;
    };
    lc("e1e2e_mmember_to_row_param_under_owns",
       "f(struct U *p) requires(owns(*p)) { zz(p->m, 2); } with a reads_range callee: the member's rows are borrowed and returned",
       [=](Kit& k)
       {
           p_row_call(k, true, 0, "2");
       });
    lc("e1e2e_mmember_to_row_param_under_owns_gives_owns",
       "the same with an owns_range callee that returns the rows: the structure is held again at the return",
       [=](Kit& k)
       {
           p_row_call(k, true, 1, "2");
       });
    lc("e1e2e_mmember_to_row_param_under_owns_not_returned_fail",
       "the same with an owns_range callee that keeps the rows: f returns without owning *p (permission, at the return)",
       [=](Kit& k)
       {
           SrcLoc r;
           p_row_call(k, true, 2, "2", &r);
           k.fail_only("permission", r, 10U, 5U);
       });
    lc("e1e2e_mmember_to_row_param_under_owns_too_many_rows_fail",
       "zz(p->m, 3) over a member of 2 rows: the shape row at the call (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", p_row_call(k, true, 0, "3")->loc, 10U, 0U);
       });
    lc("e1e2e_mmember_to_row_param_under_reads_owns_needed_fail",
       "f holds *p only for reading and gives the rows to an owns_range callee: the marked rows at the call (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", p_row_call(k, false, 1, "2")->loc, 9U, 5U);
       });
    // ---- loops in the function of a local structure with a matrix member (item 10): the member is an array place of the loop whose one leaf is the row ----
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
    };
    // int f(void) { struct U u; zero fill of u.m; int i = 0; int j = 0;
    //   while (i < 2) inv 0 <= i <= 2 decreases 2 - i { j = 0; while (j < 3) inv 0 <= j <= 3 [&& (j >= 3 || u.m[i][j] == 0)] [&& u.m[1][2] == 0] decreases 3 - j { [u.m[i][j] = 1;] j++; } i++; }
    //   check(u.m[0][1] == <want>); return 0; }
    // `mode`: 0 the reads (the invariants keep the zeros), 1 want 1, 2 check(false) after the loops, 3 check(false) at the end of the inner body
    const auto lnest = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", k.i32)};
        Var* u {k.scell(f, "u", S)};
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        auto rd = [&](const Expr* a, const Expr* b)
        {
            return k.deref(place(k, k.addr(u), a, b));
        };
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib;
        if (c3 != nullptr)
        {
            ib.push_back(c3);
        }
        ib.push_back(inc(k, j));
        const Stmt* inner {k.whl(k.lt(k.v(j), N(k, "3")),
                                 {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3")))),
                                  k.cl(k.lor(k.ge(k.v(j), N(k, "3")), k.eq(rd(k.v(i), k.v(j)), N(k, "0")))),
                                  k.cl(k.eq(rd(N(k, "1"), N(k, "2")), N(k, "0")))},
                                 {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc(ib))};
        const Stmt* outer {k.whl(k.lt(k.v(i), N(k, "2")),
                                 {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2"))))},
                                 {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc({k.asg(j, N(k, "0")), inner, inc(k, i)}))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        const Stmt* c1 {k.chk(k.eq(rd(N(k, "0"), N(k, "1")), N(k, mode == 1 ? "1" : "0")))};
        std::vector<const Stmt*> body {k.decl(u, nullptr),
                                       k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                                       k.lstore(k.addr(u), 0, N(k, "1")),
                                       k.decl(i, N(k, "0")),
                                       k.decl(j, N(k, "0")),
                                       outer,
                                       c1};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        body.push_back(k.ret(N(k, "0")));
        f->body = k.blkc(std::move(body));
        k.done(f);
        return mode == 1 ? c1 : mode == 2 ? c2 : mode == 3 ? c3 : nullptr;
    };
    lc("e1e2e_mmember_loop_nested_reads",
       "two nested loops over a zero-filled u.m that only read: one array place for the member (its cells over two bound variables), the invariants read u.m[1][2] and u.m[i][j] (both bounds, the cell's flag), the zeros are kept",
       [=](Kit& k)
       {
           lnest(k, 0);
       });
    lc("e1e2e_mmember_loop_nested_wrong_fail", "the same loops, then check(u.m[0][1] == 1): fails at the check",
       [=](Kit& k)
       {
           k.fail("check", lnest(k, 1)->loc);
       });
    lc("e1e2e_mmember_loop_nested_vacuity", "the same loops with check(false) after them: it fails",
       [=](Kit& k)
       {
           k.fail("check", lnest(k, 2)->loc);
       });
    // int f(void) { struct U u; zero fill; u.n = 1; int i = 0; while (i < 3) inv 0 <= i <= 3 && u.m[1][2] == 0 decreases 3 - i { u.m[1][i] = 1; i++; } return 0; }
    lc("e1e2e_mmember_loop_write_loses_values_fail",
       "the loop writes u.m[1][i]: a store reaches the member's place, so u.m[1][2] == 0 is not kept (the invariant fails at the back edge, once)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.i32)};
           Var* u {k.scell(f, "u", S)};
           Var* i {k.local(f, "i", k.i32)};
           const Clause keep {k.cl(k.eq(k.deref(place(k, k.addr(u), N(k, "1"), N(k, "2"))), N(k, "0")))};
           const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "3")),
                                   {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                                   {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                   k.blkc({k.store(place(k, k.addr(u), N(k, "1"), k.v(i)), N(k, "1")), inc(k, i)}))};
           f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                             k.lstore(k.addr(u), 0, N(k, "1")), k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
           k.done(f);
           k.fail("invariant-preserved", keep.loc);
       });
    // int f(void) { struct U u; zero fill; int j = 0; while (j < 3) inv 0 <= j <= 3 && u.m[0][j] == 0 decreases 3 - j { j++; } } with the invariant read unguarded where j reaches 3
    lc("e1e2e_mmember_loop_invariant_second_index_fail",
       "the invariant u.m[0][j] == 0 with j running to 3: the second index against the row's length fails in the invariant (out-of-bounds at the read)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.i32)};
           Var* u {k.scell(f, "u", S)};
           Var* j {k.local(f, "j", k.i32)};
           const Expr* rd {k.deref(place(k, k.addr(u), N(k, "0"), k.v(j)))};
           const Stmt* loop {
               k.whl(k.lt(k.v(j), N(k, "3")),
                     {k.cl(k.land(k.ge(k.v(j), N(k, "0")), k.le(k.v(j), N(k, "3")))), k.cl(k.eq(rd, N(k, "0")))},
                     {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc({inc(k, j)}))};
           f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), k.decl(j, N(k, "0")),
                             loop, k.ret(N(k, "0"))});
           k.done(f);
           k.fail_only("out-of-bounds", rd->loc, 2U, 1U, "whose second index may be outside");
       });
    lc("e1e2e_mmember_loop_nested_body_vacuity",
       "the same loops with check(false) at the end of the inner body: it fails",
       [=](Kit& k)
       {
           k.fail("check", lnest(k, 3)->loc);
       });
}

void define_part_e_matrix_member_refusals()
{
    // struct T { int m[2][3]; int n; }: a matrix member (the member first, so a cell leaf is not the first leaf)
    auto TM = [](Kit& k)
    {
        return k.strukt("T", {{"m", e1e2e_matrix(k)}, {"n", k.i32}});
    };
    auto fp = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // ---- question 348: a structure with a matrix member stays rejected at each of item 15's six positions (the array member's element is a structure with an array member); each is the copy of Part D's case ----
    const char* const kAgg {"decision 0064 item 15, question 348"};
    rc("e1e2e_rej_mmember_owns_block", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p) requires(owns_block(p)); struct T { int m[2][3]; int n; }
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_marker_on_a_pointer", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::must_init]]) requires(owns(*p)) ensures(owns(*p))
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::MustInit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_matrix_beside_an_owning_member", kAgg,
       [=](Kit& k)
       {
           // struct R { int m[2][3]; int *q [[cerastes::owning]]; }
           StructType* R {k.strukt("R", {{"m", e1e2e_matrix(k)}, {"q", k.ptr(k.i32)}})};
           R->members[1].owning = true;
       });
    rc("e1e2e_rej_mmember_owning_member_to_a_matrix_structure", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q [[cerastes::owning]]; int z; }
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}, {"z", k.i32}})};
           R->members[0].owning = true;
       });
    rc("e1e2e_rej_mmember_owning_global", kAgg,
       [=](Kit& k)
       {
           // struct T *g [[cerastes::owning]]; struct T { int m[2][3]; int n; }
           StructType* T {TM(k)};
           Global* g {k.prog.add_global("g", k.ptr(k.st(T)), k.at())};
           g->init_kind = Global::Init::Null;
           g->owning = true;
           Function* f {k.fn("f", k.vd)};
           f->body = k.blkc({});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_owning_local", kAgg,
       [=](Kit& k)
       {
           // void f(void) { struct T *q [[cerastes::owning]] = NULL; }
           StructType* T {TM(k)};
           const Type TP {k.ptr(k.st(T))};
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", TP)};
           q->owning = true;
           f->body = k.blkc({k.decl(q, k.nul(TP))});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_owning_formal_implicit_atom", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::owning]]): the implicit atom owns(*p) of an owning formal
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           const Expr* a {k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at())};
           const_cast<Expr*>(a)->implicit_atom = true;
           k.req(f, a);
           f->body = k.blkc({});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_atom_through_a_heap_read", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(owns(*r->q))
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.deref(k.mem(k.F(f, 0), 0)), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_element_read_through_a_heap_read",
       "an element read in a contract of an array member whose structure is not a pointer parameter",
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(r->q->m[1][1] == 0)
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           const Expr* loaded {k.deref(k.mem(k.F(f, 0), 0))};
           const Expr* row {k.b.elem_addr(k.mem(loaded, 0), k.num(k.i32, "1"), k.at())};
           k.req(f, k.eq(k.deref(k.b.elem_addr(row, k.num(k.i32, "1"), k.at())), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_mmember_partnerless_result", kAgg,
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) (no body)
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    rc("e1e2e_rej_mmember_ref_to_uninit_local", kAgg,
       [=](Kit& k)
       {
           // struct T *p [[cerastes::ref_to_uninit]] = null;
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd)};
           Var* p {k.local(f, "p", k.ptr(k.st(T)))};
           p->ref_to_uninit = true;
           f->body = k.blk({k.decl(p, k.nul(k.ptr(k.st(T))))});
           k.done(f);
       });
    // ---- the lowering's own stop at each Part D site that takes an aggregate element (the harness cannot pin an exit 3, so each is a refused program lowered anyway) ----
    ru("e1e2e_ru_mmember_block_lines_of_an_aggregate_array_structure", "the lines of an owns_block",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    ru("e1e2e_ru_mmember_block_rows_of_an_aggregate_array_structure", "the rows of a block",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           k.ens(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2e_ru_mmember_block_framing_of_an_aggregate_array_structure", "the framing of a contract read",
       [=](Kit& k)
       {
           // void f(struct T *p) requires(owns_block(p) && p->m[0][0] == 0)
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           const Expr* el {k.b.elem_addr(k.mem(fp(k, f, 0), 0), k.num(k.i32, "0"), k.at())};
           k.req(f, k.land(k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()),
                           k.eq(k.deref(k.b.elem_addr(el, k.num(k.i32, "0"), k.at())), k.num(k.i32, "0"))));
           f->body = k.blk({});
           k.done(f);
       });
    ru("e1e2e_ru_mmember_block_call_record_of_an_aggregate_array_structure",
       "the sequencing record of a call's block site",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
           Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(T))}})};
           k.req(g, k.b.atom(AtomKind::OwnsBlock, fp(k, g, 0), -1, k.at()));
           g->body = k.blk({k.es(k.call(f, {k.pv(g, 0)}))});
           k.done(g);
       });
    ru("e1e2e_ru_mmember_marked_atom_array_part_of_an_aggregate", "the array part of a marked or partnerless atom",
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::ref_to_uninit]]) requires(owns(*p)) ensures(owns(*p)) {}
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2e_ru_mmember_marked_return_rows_of_an_aggregate", "the return rows of a marked or partnerless atom",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2e_ru_mmember_marked_call_rows_of_an_aggregate", "the call rows of an atom under an initialization marker",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
           Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(T))}})};
           k.req(g, k.b.atom(AtomKind::Owns, fp(k, g, 0), 0, k.at()));
           k.ens(g, k.b.atom(AtomKind::Owns, fp(k, g, 0), 0, k.at()));
           g->body = k.blkc({k.es(k.call(f, {k.pv(g, 0)}))});
           k.done(g);
       });
    ru("e1e2e_ru_mmember_partnerless_array_part_of_an_aggregate", "the array part of a marked or partnerless atom",
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) (no body)
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    ru("e1e2e_ru_mmember_partnerless_return_rows_of_an_aggregate", "the return rows of a marked or partnerless atom",
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) { return null; }
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           f->body = k.blk({k.ret(k.nul(k.ptr(k.st(T))))});
           k.done(f);
       });
    ru("e1e2e_ru_mmember_contract_read_through_a_heap_read_of_an_aggregate",
       "the base of an element read in a contract is not a pointer parameter",
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(r->q->m[1][1] == 0)
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           const Expr* loaded {k.deref(k.mem(k.F(f, 0), 0))};
           const Expr* row {k.b.elem_addr(k.mem(loaded, 0), k.num(k.i32, "1"), k.at())};
           k.req(f, k.eq(k.deref(k.b.elem_addr(row, k.num(k.i32, "1"), k.at())), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    // a written atom in a loop invariant on a structure with a matrix member, or on the matrix member (the sixth position)
    const auto written_m = [=](Kit& k, bool member_array)
    {
        StructType* S {TM(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
        k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* atom {member_array ? k.b.atom(AtomKind::Owns, k.mem(k.pv(f, 0), 0), 1, k.at())
                                       : k.b.atom(AtomKind::Owns, k.pv(f, 0), 0, k.at())};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.num(k.i32, "4")), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "4"), atom))},
                              {k.cl(k.sub(k.num(k.i32, "4"), k.v(i)))},
                              k.blkc({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))};
        f->body = k.blkc({k.decl(i, k.num(k.i32, "0")), lp});
        k.done(f);
    };
    rc("e1e2e_rej_mmember_written_owns_structure",
       "a permission atom on a structure with an array member of structures or arrays in a loop invariant",
       [=](Kit& k)
       {
           written_m(k, false);
       });
    rc("e1e2e_rej_mmember_written_owns_member", "in a loop invariant",
       [=](Kit& k)
       {
           written_m(k, true);
       });
    ru("e1e2e_ru_mmember_written_owns_structure",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written_m(k, false);
       });
    ru("e1e2e_ru_mmember_written_owns_member",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written_m(k, true);
       });
}

int check_value_cells()
{
    int bad {0};
    auto expect = [&](bool ok, const char* what)
    {
        if (!ok)
        {
            std::fprintf(stderr, "FAIL (value cells) %s\n", what);
            ++bad;
        }
    };
    Kit k;
    const Type row {e1e2e_row(k)};
    const Type matrix {e1e2e_matrix(k)};
    // struct E { int a[2]; int n; }; struct S { struct E v[2]; };  the order of the one enumeration
    StructType* E {k.strukt("E", {{"a", k.prog.array_of(k.i32, 2U)}, {"n", k.i32}})};
    StructType* S {k.strukt("S", {{"v", k.prog.array_of(k.st(E), 2U)}})};
    {
        bool complete {false};
        const std::vector<ValueCell> cs {value_cells(k.st(S), "s", false, &complete)};
        const char* want[] {"s.v[0].a[0]", "s.v[0].a[1]", "s.v[0].n", "s.v[1].a[0]", "s.v[1].a[1]", "s.v[1].n"};
        bool same {complete && cs.size() == 6U};
        for (size_t i {0U}; same && i < 6U; ++i)
        {
            same = cs[i].text == want[i] && cs[i].type == k.i32;
        }
        expect(same, "value_cells of struct S: depth first, members in declaration order, elements in index order");
        if (cs.size() == 6U)
        {
            expect(
                cs[1U].steps == std::vector<ValueStep> {{false, 0U}, {true, 0U}, {false, 0U}, {true, 1U}} &&
                    cs[1U].indexes == 2U && cs[1U].aggregate_below,
                "the steps of s.v[0].a[1]: member v, element 0, member a, element 1; two index steps, inside an array of aggregates");
            expect(cs[2U].steps == std::vector<ValueStep> {{false, 0U}, {true, 0U}, {false, 1U}} &&
                       cs[2U].indexes == 1U && cs[2U].aggregate_below,
                   "the steps of s.v[0].n: one index step, followed by a member");
        }
    }
    {
        // a matrix is row major
        const std::vector<ValueCell> cs {value_cells(matrix, "m")};
        bool same {cs.size() == 6U};
        const char* want[] {"m[0][0]", "m[0][1]", "m[0][2]", "m[1][0]", "m[1][1]", "m[1][2]"};
        for (size_t i {0U}; same && i < 6U; ++i)
        {
            same = cs[i].text == want[i] && cs[i].indexes == 2U && cs[i].aggregate_below;
        }
        expect(same, "value_cells of int[2][3]: row major, two index steps each");
    }
    {
        // a scalar is one cell with no steps; an array of scalars is one index step, last, not inside an array of aggregates
        const std::vector<ValueCell> one {value_cells(k.i32, "x")};
        expect(one.size() == 1U && one[0U].steps.empty() && one[0U].text == "x" && one[0U].indexes == 0U &&
                   !one[0U].aggregate_below,
               "a scalar is one cell");
        const std::vector<ValueCell> r {value_cells(row, "r")};
        expect(r.size() == 3U && r[2U].text == "r[2]" && r[2U].indexes == 1U && !r[2U].aggregate_below,
               "an array of scalars: the index step is last");
        // `arrow`: the first member of a pointer root is spelled `->`
        const std::vector<ValueCell> ar {value_cells(k.st(S), "p", true)};
        expect(!ar.empty() && ar[0U].text == "p->v[0].a[0]", "arrow spells the first member `->`");
    }
    {
        // the cut: a type of more cells than max_cells is cut and the list says so
        bool complete {true};
        const std::vector<ValueCell> cs {value_cells(k.prog.array_of(k.i32, 100000U), "a", false, &complete, 10U)};
        expect(!complete && cs.size() == 10U, "an enormous array is cut at max_cells and reported incomplete");
    }
    {
        // for the types of depth 0 and 1 the one enumeration is struct_value_leaves' list, cell for cell: types of records 0043, 0044, 0063 and Part C2
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.bl}})};
        StructType* T {k.strukt("T", {{"p", k.i32}, {"a", k.prog.array_of(k.i32, 2U)}})};
        StructType* U {k.strukt("U", {{"t", k.st(T)}, {"q", k.prog.array_of(k.ptr(k.i32), 3U)}, {"b", k.bl}})};
        StructType* V {k.strukt("V", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}})};
        StructType* X {k.strukt("X", {{"w", k.prog.array_of(k.st(P), 2U)}, {"t", k.st(T)}, {"n", k.i64}})};
        for (const StructType* st : {P, T, U, V, X})
        {
            const std::vector<StructLeaf> leaves {struct_value_leaves(*st, "s")};
            const std::vector<ValueCell> cells {value_cells(k.st(st), "s")};
            bool same {leaves.size() == cells.size()};
            for (size_t i {0U}; same && i < leaves.size(); ++i)
            {
                std::vector<int> members;
                int elem {-1};
                for (const ValueStep& s : cells[i].steps)
                {
                    if (s.index)
                    {
                        elem = static_cast<int>(s.value);
                    }
                    else
                    {
                        members.push_back(static_cast<int>(s.value));
                    }
                }
                same = leaves[i].text == cells[i].text && leaves[i].type == cells[i].type &&
                       leaves[i].path == members && leaves[i].elem == elem && !leaves[i].aggregate_element;
            }
            expect(same, "value_cells of a type of depth 0 or 1 is struct_value_leaves' list");
            expect(type_index_depth(k.st(st)) <= 1U, "the types above are of depth 0 or 1");
            expect(
                value_index_cells(k.st(st)) == e1e2e_index_cells(*st),
                "value_index_cells is the sum of the width of the array members (the formula before the one definition)");
        }
    }
    {
        // the types of Part E: the depth, and the two counts
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 4U)}, {"n", k.i32}})};
        StructType* T {k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}})};
        StructType* M {k.strukt("M", {{"m", matrix}, {"a", k.prog.array_of(k.i32, 2U)}, {"n", k.i32}})};
        expect(type_index_depth(k.i32) == 0U && type_index_depth(row) == 1U && type_index_depth(matrix) == 2U &&
                   type_index_depth(k.prog.array_of(matrix, 2U)) == 3U,
               "type_index_depth of a scalar, a row, a matrix, a cube");
        expect(type_index_depth(k.st(W)) == 1U && type_index_depth(k.st(T)) == 2U && type_index_depth(k.st(M)) == 2U,
               "type_index_depth of struct W (1), struct T (2: v[i].a[j]) and struct M (2: a matrix member)");
        // the by-value count: every cell reached through an index step; the copy count: the cells inside an array of aggregates
        expect(value_index_cells(k.st(W)) == 4U && copied_index_cells(k.st(W)) == 0U,
               "W: the 4 elements of its array member count by value, none for a copy (copy_arr_<t> copies it)");
        expect(value_index_cells(k.st(T)) == 10U && copied_index_cells(k.st(T)) == 10U,
               "T { W v[2]; }: 10 cells by value and in a copy (the record's own example)");
        expect(
            value_index_cells(k.st(M)) == 8U && copied_index_cells(k.st(M)) == 6U,
            "M { int m[2][3]; int a[2]; int n; }: a scalar array beside the matrix counts by value (8) and not in a copy (6)");
        expect(value_index_cells(matrix) == 6U && copied_index_cells(matrix) == 6U && copied_index_cells(row) == 0U,
               "a matrix is 6 cells by value and in a copy, a row 3 and 0");
        // the value view of a type of depth 2 or more IS value_cells (slice 4a), cell for cell: the same text, type and steps, never flagged, and `elem` the first index step
        for (const StructType* st : {T, M})
        {
            const std::vector<StructLeaf> leaves {struct_value_leaves(*st, "s")};
            const std::vector<ValueCell> cells {value_cells(k.st(st), "s")};
            bool same {!leaves.empty() && leaves.size() == cells.size()};
            for (size_t i {0U}; same && i < leaves.size(); ++i)
            {
                int first {-1};
                for (const ValueStep& step : cells[i].steps)
                {
                    if (step.index && first < 0)
                    {
                        first = static_cast<int>(step.value);
                    }
                }
                same = leaves[i].text == cells[i].text && leaves[i].type == cells[i].type &&
                       leaves[i].steps == cells[i].steps && leaves[i].elem == first && !leaves[i].aggregate_element;
            }
            expect(same,
                   "the value view of a type of depth 2 is value_cells' list: text, type and steps cell for cell");
        }
        expect(struct_value_leaves(*T).size() == 10U && struct_value_leaves(*M).size() == 9U,
               "T { W v[2]; } has 10 leaves in the value view, M { int m[2][3]; int a[2]; int n; } 9");
    }
    {
        // Builder::elem_addr (record 0064 items 4 and 13): the element of the address of an array object is the array's element; the element of a pointer VALUE of a pointer-to-row type is the
        // ROW, whose address has the pointer's own type
        Function* f {k.fn("f", k.vd, {{"a", k.ptr(row)}})};
        k.done(f);
        Var* m {f->new_local("m", matrix, k.at())};
        f->mark_address_taken(m);
        const Expr* i {k.num(k.i32, "1")};
        const Expr* via_param {k.b.elem_addr(k.pv(f, 0), i, k.at())};
        expect(via_param->type == k.ptr(row),
               "a[i] of a row parameter is the address of the row: the pointer's own type");
        const Expr* via_formal {k.b.elem_addr(k.F(f, 0), i, k.at())};
        expect(via_formal->type == k.ptr(row), "the same through the formal of a contract");
        const Expr* named_row {k.b.elem_addr(k.addr(m), i, k.at())};
        expect(named_row->type == k.ptr(row),
               "m[i] of a local matrix is the address of a row, typed pointer to int[3]");
        const Expr* cell {k.b.elem_addr(named_row, i, k.at())};
        expect(cell->type == k.ptr(k.i32), "m[i][j] is the address of a cell: the row's element");
        const Expr* via_param_cell {k.b.elem_addr(via_param, i, k.at())};
        expect(via_param_cell->type == k.ptr(k.i32), "a[i][j] of a row parameter: the element of the row's address");
        expect(array_object_address(*named_row) && array_object_address(*k.addr(m)) &&
                   !array_object_address(*k.pv(f, 0)) && !array_object_address(*k.F(f, 0)),
               "array_object_address: AddrOf and ElemAddr are array objects, a parameter is a pointer value");
        expect(place_text(*cell) == "m[1][1]", "place_text of m[i][j] over a local matrix");
    }
    return bad;
}

} // namespace

// ---- decision 0064 items 7 and 13 (Part E, slice 3b-4; question 362): the contract reads of a matrix MEMBER, `p->m[e][e2]`, `gt.m[e][e2]` and `r->m[e][e2]`, framed by the structure atom's array part or by the member's atom ----
void define_part_e_matrix_member_contract_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct U { int n; int m[2][3]; }
    auto U = [](Kit& k) -> StructType*
    {
        return k.strukt("U", {{"n", k.i32}, {"m", e1e2e_matrix(k)}});
    };
    auto PU = [](Kit& k, StructType* S)
    {
        return k.ptr(k.st(S));
    };
    // the read base->m[i][j] for a pointer expression `base`
    auto mread = [=](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.deref(k.b.elem_addr(k.b.elem_addr(k.mem(base, 1), N(k, i), k.at()), N(k, j), k.at()));
    };
    // the atom of `shape` on the formal p: 0 owns(*p), 1 reads(*p), 2 owns(p->m), 3 reads(p->m), 4 none, 5 owns(p->n)
    auto patom = [=](Kit& k, Function* f, int shape) -> const Expr*
    {
        const Expr* p {k.b.text(k.F(f, 0), "p")};
        switch (shape)
        {
            case 0:
                return k.b.atom(AtomKind::Owns, p, 0, k.at());
            case 1:
                return k.b.atom(AtomKind::Reads, p, 0, k.at());
            case 2:
                return k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), 1), 0, k.at());
            case 3:
                return k.b.atom(AtomKind::Reads, k.mem(k.b.text(k.F(f, 0), "p"), 1), 0, k.at());
            case 5:
                return k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), 0), 0, k.at());
            default:
                return nullptr;
        }
    };
    auto with = [=](Kit& k, const Expr* atom, const Expr* c) -> const Expr*
    {
        return atom != nullptr ? k.land(atom, c) : c;
    };
    // int f(struct U *p) requires(<atom> && p->m[ri][rj] == 4) ensures(<atom> && r == p->m[ei][ej]) { return p->m[1][0]; }
    // main: struct U u = zero fill of the member; u.m[1][0] = 4; int r = f(&u); check(r == u.m[1][0]);
    const auto lread = [=](Kit& k, int shape, const char* ri, const char* rj, const char* ei, const char* ej,
                           SrcLoc* call_loc = nullptr, SrcLoc* req_loc = nullptr, SrcLoc* ens_loc = nullptr,
                           SrcLoc* ret_loc = nullptr, bool divide = false) -> const Stmt*
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", k.i32, {{"p", PU(k, S)}})};
        const Expr* rd1 {mread(k, k.F(f, 0), ri, rj)};
        const Expr* rd2 {mread(k, k.F(f, 0), ei, ej)};
        if (req_loc != nullptr)
        {
            *req_loc = rd1->loc;
        }
        const Expr* body_read {
            k.deref(k.b.elem_addr(k.b.elem_addr(k.mem(k.pv(f, 0), 1), N(k, "1"), k.at()), N(k, "0"), k.at()))};
        k.req(f, with(k, patom(k, f, shape), divide ? k.ne(rd1, N(k, "0")) : k.eq(rd1, N(k, "4"))));
        const Expr* ens_value {divide ? k.div(N(k, "10"), rd2) : rd2};
        if (ens_loc != nullptr)
        {
            *ens_loc = ens_value->loc;
        }
        k.ens(f, with(k, patom(k, f, shape), k.eq(k.R(f), ens_value)));
        const Stmt* ret {k.ret(divide ? k.div(N(k, "10"), body_read) : body_read)};
        if (ret_loc != nullptr)
        {
            *ret_loc = ret->loc;
        }
        f->body = k.blk({ret});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* u {k.scell(m, "u", S)};
        Var* r {k.local(m, "r", k.i32)};
        const Expr* callx {k.call(f, {k.addr(u)})};
        if (call_loc != nullptr)
        {
            *call_loc = callx->loc;
        }
        const Expr* cell {k.b.elem_addr(k.b.elem_addr(k.mem(k.addr(u), 1), N(k, "1"), k.at()), N(k, "0"), k.at())};
        const Stmt* c {
            divide ? k.chk(k.yes())
                   : k.chk(k.eq(k.v(r), k.deref(k.b.elem_addr(k.b.elem_addr(k.mem(k.addr(u), 1), N(k, ei), k.at()),
                                                              N(k, ej), k.at()))))};
        m->body =
            k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), k.lstore(k.addr(u), 0, N(k, "1")),
                    k.store(cell, N(k, "4")), k.decl(r, callx), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_mmember_contract_read_owns_whole",
       "requires owns(*p) && p->m[1][0] == 4, ensures owns(*p) && r == p->m[1][0]: framed by the structure atom's array part; the caller's check of the result against u.m[1][0] holds",
       [=](Kit& k)
       {
           lread(k, 0, "1", "0", "1", "0");
       });
    lc("e1e2e_mmember_contract_read_reads_whole", "the same under reads(*p)",
       [=](Kit& k)
       {
           lread(k, 1, "1", "0", "1", "0");
       });
    lc("e1e2e_mmember_contract_read_owns_member", "the same under owns(p->m), the member's own atom",
       [=](Kit& k)
       {
           lread(k, 2, "1", "0", "1", "0");
       });
    lc("e1e2e_mmember_contract_read_reads_member", "the same under reads(p->m)",
       [=](Kit& k)
       {
           lread(k, 3, "1", "0", "1", "0");
       });
    lc("e1e2e_mmember_contract_read_wrong_cell_fail",
       "requires p->m[1][0] == 4 but ensures r == p->m[1][1] under owns(*p): the body returns p->m[1][0], the postcondition fails",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           SrcLoc ret;
           lread(k, 0, "1", "0", "1", "1", &call, &req, nullptr, &ret);
           k.fail("postcondition", ret);
       });
    lc("e1e2e_mmember_contract_read_wrong_row_fail",
       "the same with the ensures reading p->m[0][0]: the postcondition fails",
       [=](Kit& k)
       {
           SrcLoc ret;
           lread(k, 0, "1", "0", "0", "0", nullptr, nullptr, nullptr, &ret);
           k.fail("postcondition", ret);
       });
    lc("e1e2e_mmember_contract_read_unframed_fail",
       "no atom on p: the read p->m[1][0] in the requires is held by nothing (permission at the read); main's call fails the precondition",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           SrcLoc ens;
           lread(k, 4, "1", "0", "1", "0", &call, &req, &ens);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_mmember_contract_read_other_member_atom_fail",
       "owns(p->n) frames the member n alone: the read of the matrix member is held by nothing (permission)",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           SrcLoc ens;
           lread(k, 5, "1", "0", "1", "0", &call, &req, &ens);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_mmember_contract_read_second_index_outside_fail",
       "requires owns(*p) && p->m[1][3] == 4: the second index is outside the row of three: no cell of the member is that place, permission",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           lread(k, 0, "1", "3", "1", "0", &call, &req);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_mmember_contract_read_tie_false_alarm_fail",
       "requires reads(*p) && p->m[1][0] != 0, ensures r == 10 / p->m[1][0]: f__wf does not tie the entry and exit ghosts of a matrix member's cell (0063 item 7's N1, question 345's listed false alarm): division-by-zero at the ensures, a fail-closed false alarm, pinned",
       [=](Kit& k)
       {
           SrcLoc ens;
           lread(k, 1, "1", "0", "1", "0", nullptr, nullptr, &ens, nullptr, true);
           k.fail("division-by-zero", ens);
       });
    // struct W { int m[2][3]; int m2[2][3]; }: two matrix members; void f(struct W *p) requires(reads(*p) && p->m[0][1] == 3 && p->m[1][0] == 4 && p->m2[1][0] == 5 && p->m[1][1] == 6 && p->m[0][1] == 3) { }
    lc("e1e2e_mmember_contract_reads_keys",
       "five reads of three distinct cells of m and one of m2 under reads(*p): the member, the first and the second index are each part of the key (p->m[1][0] and p->m2[1][0] are two ghosts): five ghosts, none unframed",
       [=](Kit& k)
       {
           StructType* S {k.strukt("W", {{"m", e1e2e_matrix(k)}, {"m2", e1e2e_matrix(k)}})};
           Function* f {k.fn("f", k.vd, {{"p", PU(k, S)}})};
           auto rd = [&](int member, const char* i, const char* j)
           {
               return k.deref(k.b.elem_addr(k.b.elem_addr(k.mem(k.F(f, 0), member), N(k, i), k.at()), N(k, j), k.at()));
           };
           k.req(f, k.land(k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()),
                           k.land(k.land(k.eq(rd(0, "0", "1"), N(k, "3")), k.eq(rd(0, "1", "0"), N(k, "4"))),
                                  k.land(k.land(k.eq(rd(1, "1", "0"), N(k, "5")), k.eq(rd(0, "1", "1"), N(k, "6"))),
                                         k.eq(rd(0, "0", "1"), N(k, "3"))))));
           f->body = k.blk({});
           k.done(f);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   std::set<std::string> out;
                   for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
                   {
                       size_t end {at + 4U};
                       while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
                       {
                           ++end;
                       }
                       out.insert(vpr.substr(at, end - at));
                   }
                   return out.size() == 4U ? ""
                                           : "the ghosts of the six reads: expected 4 (one per distinct cell), found " +
                                                 std::to_string(out.size());
               });
       });
    // struct U *f(struct U *p) requires(owns(*p) && p->m[0][0] == 3) ensures(owns(*p) && result == p && result->m[0][0] == 3) { return p; }
    // main: struct U u; zero fill; u.n = 1; u.m[0][0] = 3; struct U *q = f(&u); check(q->m[0][0] == 3)
    const auto lresult = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S {U(k)};
        Function* f {k.fn("f", PU(k, S), {{"p", PU(k, S)}})};
        const Expr* pr {k.b.text(k.F(f, 0), "p")};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, pr, 0, k.at()), k.eq(mread(k, k.F(f, 0), "0", "0"), N(k, "3"))));
        k.ens(f, k.land(k.land(k.b.atom(AtomKind::Owns, pr, 0, k.at()), k.eq(k.R(f), k.F(f, 0))),
                        k.eq(mread(k, k.R(f), "0", "0"), N(k, wrong ? "4" : "3"))));
        const Stmt* ret {k.ret(k.pv(f, 0))};
        f->body = k.blk({ret});
        k.done(f);
        return ret;
    };
    lc("e1e2e_mmember_contract_read_result",
       "an ensures that reads result->m[0][0] under owns(*p) && result == p: the result is the parameter, whose atom frames the read (the structure's array part); the body returns p, which has not been written",
       [=](Kit& k)
       {
           lresult(k, false);
       });
    lc("e1e2e_mmember_contract_read_result_wrong_fail",
       "the same ensuring result->m[0][0] == 4 where the requires said 3: the postcondition fails at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", lresult(k, true)->loc);
       });
    // ---- a global structure: gu.m[e][e2] under reads(gu) / owns(gu) ----
    auto gu_global = [=](Kit& k, StructType* S) -> Global*
    {
        Global* g {k.prog.add_global("gu", k.st(S), k.at(), nullptr)};
        GlobalLeaf ln;
        ln.kind = GlobalInit::Value;
        ln.init = k.num(k.i32, "7");
        ln.value = "7";
        ln.loc = k.at();
        g->leaves.push_back(ln);
        GlobalLeaf lm;
        lm.kind = GlobalInit::Value;
        lm.rest = ArrayRest::Zero;
        lm.loc = k.at();
        for (const std::vector<const char*>& r : std::vector<std::vector<const char*>> {{"1", "2", "3"}, {"4"}})
        {
            GlobalLeaf row;
            row.kind = GlobalInit::Value;
            row.rest = ArrayRest::Zero;
            row.loc = k.at();
            for (const char* v : r)
            {
                GlobalLeaf c;
                c.kind = GlobalInit::Value;
                c.value = v;
                c.init = k.num(k.i32, v);
                c.loc = k.at();
                row.elems.push_back(c);
            }
            lm.elems.push_back(row);
        }
        g->leaves.push_back(lm);
        return g;
    };
    // int f(void) requires(<atom on gu> && gu.m[1][0] == <want>) ensures(<atom> && r == gu.m[1][0]) { return gu.m[1][0]; }   int main() { return f() == gu.m[1][0] ? 0 : 1; } with a check
    const auto lglobal = [=](Kit& k, int shape, const char* want, SrcLoc* call_loc = nullptr,
                             SrcLoc* req_loc = nullptr) -> const Stmt*
    {
        StructType* S {U(k)};
        Global* g {gu_global(k, S)};
        auto gread = [&]
        {
            return k.deref(k.b.elem_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), N(k, "1"), k.at()),
                                         N(k, "0"), k.at()));
        };
        Function* f {k.fn("f", k.i32)};
        const Expr* rd1 {gread()};
        if (req_loc != nullptr)
        {
            *req_loc = rd1->loc;
        }
        auto atom = [&]() -> const Expr*
        {
            return shape == 3
                       ? nullptr
                       : k.b.global_atom(shape == 0 ? AtomKind::Reads : AtomKind::Owns,
                                         shape == 2 ? k.mem(k.b.global_addr(g, k.at()), 1) : k.b.global_addr(g, k.at()),
                                         0, k.at(), false);
        };
        k.req(f, with(k, atom(), k.eq(rd1, N(k, want))));
        k.ens(f, with(k, atom(), k.eq(k.R(f), gread())));
        f->body = k.blk({k.ret(gread())});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* rv {k.local(m, "rv", k.i32)};
        const Expr* callx {k.call(f, {})};
        if (call_loc != nullptr)
        {
            *call_loc = callx->loc;
        }
        const Stmt* c {k.chk(k.eq(k.v(rv), gread()))};
        m->body = k.blkc({k.decl(rv, callx), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_gmember_contract_read_reads",
       "requires reads(gu) && gu.m[1][0] == 4, ensures reads(gu) && r == gu.m[1][0]: framed by the global structure's atom; program_start gives gu.m[1][0] == 4",
       [=](Kit& k)
       {
           lglobal(k, 0, "4");
       });
    lc("e1e2e_gmember_contract_read_owns", "the same under owns(gu)",
       [=](Kit& k)
       {
           lglobal(k, 1, "4");
       });
    lc("e1e2e_gmember_contract_read_member_atom", "the same under owns(gu.m), the member's own atom",
       [=](Kit& k)
       {
           lglobal(k, 2, "4");
       });
    lc("e1e2e_gmember_contract_read_wrong_value_fail",
       "requires gu.m[1][0] == 5: main's gu.m[1][0] is 4 from program_start: the precondition fails at the call",
       [=](Kit& k)
       {
           SrcLoc call;
           lglobal(k, 0, "5", &call);
           k.fail("precondition", call);
       });
    lc("e1e2e_gmember_contract_read_unframed_fail",
       "no atom on gu: the read gu.m[1][0] in the requires is held by nothing (permission), and so is the call's precondition",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           lglobal(k, 3, "4", &call, &req);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_mmember_contract_read_owns_whole_vacuity_fail",
       "the same positive as e1e2e_mmember_contract_read_owns_whole with check(false) after the call: the contract's facts are satisfiable, the check is reached and fails",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.i32, {{"p", PU(k, S)}})};
           k.req(f, k.land(patom(k, f, 0), k.eq(mread(k, k.F(f, 0), "1", "0"), N(k, "4"))));
           k.ens(f, k.land(patom(k, f, 0), k.eq(k.R(f), mread(k, k.F(f, 0), "1", "0"))));
           f->body = k.blk({k.ret(N(k, "4"))});
           k.done(f);
           Function* m {k.fn("main", k.i32)};
           Var* u {k.scell(m, "u", S)};
           const Expr* cell {k.b.elem_addr(k.b.elem_addr(k.mem(k.addr(u), 1), N(k, "1"), k.at()), N(k, "0"), k.at())};
           const Stmt* c {k.chk(k.no())};
           m->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()),
                             k.lstore(k.addr(u), 0, N(k, "1")), k.store(cell, N(k, "4")), k.es(k.call(f, {k.addr(u)})),
                             c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    // int f(struct U *p) requires(reads(*p) && p->m[1][2] >= 0) ensures(reads(*p) && r == 0 && cerastes_old(p->m[1][2]) >= 0) decreases(p->m[1][2]) { return 0; }
    lc("e1e2e_mmember_contract_read_ensures_old_decreases",
       "p->m[1][2] in a requires, under old in an ensures and in a decreases: each is a contract read, framed by reads(*p)",
       [=](Kit& k)
       {
           StructType* S {U(k)};
           Function* f {k.fn("f", k.i32, {{"p", PU(k, S)}})};
           k.req(f, k.land(patom(k, f, 1), k.ge(mread(k, k.F(f, 0), "1", "2"), N(k, "0"))));
           k.ens(f, k.land(patom(k, f, 1),
                           k.land(k.eq(k.R(f), N(k, "0")), k.ge(k.old(mread(k, k.F(f, 0), "1", "2")), N(k, "0")))));
           k.dec(f, mread(k, k.F(f, 0), "1", "2"));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    rc("e1e2e_rej_gmember_static_contract_read", "its name is not in scope there",
       [=](Kit& k)
       {
           // void f(void) { static struct U su; } with requires(su.m[1][0] == 0)
           StructType* S {U(k)};
           Function* f {k.fn("f", k.vd)};
           Global* g {k.prog.add_global("su", k.st(S), k.at(), f)};
           k.req(f, k.eq(k.deref(k.b.elem_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 1), N(k, "1"), k.at()),
                                               N(k, "0"), k.at())),
                         N(k, "0")));
           f->body = k.blk({});
           k.done(f);
       });
}

// ---- decision 0064 items 3, 5, 6 and 13 (Part E, slice 3c): an array of structures whose element has an array member of scalars, `v[i].a[j]`; 3c-1 an automatic local ----
void define_part_e_struct_array_leaf_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct P { int n; int a[3]; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    // struct P v[n];
    auto arr = [](Kit& k, Function* f, StructType* P, unsigned n, bool zero)
    {
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), n), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        return v;
    };
    auto elem = [](Kit& k, Var* v, const Expr* i)
    {
        return k.b.elem_addr(k.addr(v), i, k.at());
    };
    // v[i].a[j] as a place and as a read; v[i].n
    auto place = [=](Kit& k, Var* v, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.member_addr(elem(k, v, i), 1, k.at()), j, k.at());
    };
    auto read = [=](Kit& k, Var* v, const Expr* i, const Expr* j)
    {
        return k.deref(place(k, v, i, j));
    };
    auto nplace = [=](Kit& k, Var* v, const Expr* i)
    {
        return k.b.member_addr(elem(k, v, i), 0, k.at());
    };
    // void f(void) { struct P v[2]; v[0].n = 7; v[1].a[2] = 5; check(v[1].a[2] == <want>); }
    const auto local = [=](Kit& k, const char* want, bool zero) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd)};
        Var* v {arr(k, f, P, 2U, zero)};
        const Stmt* c {k.chk(k.eq(read(k, v, N(k, "1"), N(k, "2")), N(k, want)))};
        f->body = k.blkc({k.decl(v, nullptr), k.store(nplace(k, v, N(k, "0")), N(k, "7")),
                          k.store(place(k, v, N(k, "1"), N(k, "2")), N(k, "5")), c});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_sarr_local_rw", "a local array of structures with an array member: v[1].a[2] written and read back",
       [=](Kit& k)
       {
           local(k, "5", false);
       });
    lc("e1e2e_sarr_local_wrong_fail", "the same: the check that v[1].a[2] is 6 fails",
       [=](Kit& k)
       {
           k.fail("check", local(k, "6", false));
       });
    lc("e1e2e_sarr_local_vacuity_fail",
       "the allocation's facts are satisfiable: a check(false) after the writes is reached and fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, false)};
           const Stmt* c {k.chk(k.no())};
           f->body = k.blkc({k.decl(v, nullptr), k.store(place(k, v, N(k, "1"), N(k, "2")), N(k, "5")), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    // struct P v[2] = {0}; check(v[1].a[2] == <want> && v[0].n == 0);
    const auto zero = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd)};
        Var* v {arr(k, f, P, 2U, true)};
        const Stmt* c {k.chk(k.land(k.eq(read(k, v, N(k, "1"), N(k, "2")), N(k, want)),
                                    k.eq(k.deref(nplace(k, v, N(k, "0"))), N(k, "0"))))};
        f->body = k.blkc({k.decl(v, nullptr), c});
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_sarr_zero_fill",
       "the zero fill of an array of structures with an array member: every cell of every element is zero and set",
       [=](Kit& k)
       {
           zero(k, "0");
       });
    lc("e1e2e_sarr_zero_fill_wrong_fail", "the zero fill: the check that v[1].a[2] is 1 fails",
       [=](Kit& k)
       {
           k.fail("check", zero(k, "1"));
       });
    lc("e1e2e_sarr_zero_fill_vacuity_fail",
       "the zero fill's contract is satisfiable: a check(false) after it is reached and fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           const Stmt* c {k.chk(k.no())};
           f->body = k.blkc({k.decl(v, nullptr), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    // void f(int i, int j) requires(0 <= i <= <hi_i> && 0 <= j <= <hi_j>) { struct P v[2]; v[i].a[j] = 1; }
    const auto symbolic = [=](Kit& k, const char* hi_i, const char* hi_j) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", hi_i), k.range(k.F(f, 1), k.i32, "0", hi_j)));
        Var* v {arr(k, f, P, 2U, false)};
        const Stmt* w {k.store(place(k, v, k.pv(f, 0), k.pv(f, 1)), N(k, "1"))};
        f->body = k.blkc({k.decl(v, nullptr), w});
        k.done(f);
        return w->loc;
    };
    lc("e1e2e_sarr_symbolic_index", "v[i].a[j] written for symbolic indexes in range: both bound rows hold",
       [=](Kit& k)
       {
           symbolic(k, "1", "2");
       });
    lc("e1e2e_sarr_first_index_high_fail", "v[i].a[j] with i == 2: out-of-bounds (the first index against the array)",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", symbolic(k, "2", "2"), 2U, 0U);
       });
    lc("e1e2e_sarr_second_index_high_fail",
       "v[i].a[j] with j == 3: out-of-bounds (the second index against the member's own length)",
       [=](Kit& k)
       {
           k.fail_only("out-of-bounds", symbolic(k, "1", "3"), 2U, 1U);
       });
    lc("e1e2e_sarr_uninitialized_read_fail", "v[1].a[2] read before any write: uninitialized",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, false)};
           const Expr* rd {read(k, v, N(k, "1"), N(k, "2"))};
           f->body = k.blkc({k.decl(v, nullptr), k.chk(k.eq(rd, N(k, "0")))});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    const auto exits_pin = exits_pin_min(3U);
    // int f(int c) { struct P v[2]; if (c != 0) { return 1; } { struct P w[2]; w[0].a[1] = 1; } return 0; }: every way out exhales what the allocation gave
    lc("e1e2e_sarr_scope_exits_give_what_the_allocation_gave",
       "every way out of the scope of an array of structures with an array member exhales exactly the allocation's quantifiers",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"c", k.i32}})};
           Var* v {arr(k, f, P, 2U, false)};
           Var* w {arr(k, f, P, 2U, false)};
           const Stmt* inner {k.blkc({k.decl(w, nullptr), k.store(place(k, w, N(k, "0"), N(k, "1")), N(k, "1"))})};
           f->body = k.blkc({k.decl(v, nullptr), k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({k.ret(N(k, "1"))})), inner,
                             k.ret(N(k, "0"))});
           k.done(f);
           k.text_checks.push_back(exits_pin);
       });
    // void keep(int *p, int n) requires(owns_range(p, n)); void f(void) { struct P v[2]; v[1].a[0] = 0; keep(&v[1].a[0], 3); }: the callee keeps the member's cells
    lc("e1e2e_sarr_cells_kept_fail",
       "the cells v[1].a[0..3) given to a callee that keeps them: lifetime at the scope exit (the second marked row: the array leaf's value permission)",
       [=](Kit& k)
       {
           Function* g {k.fn("keep", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.trusted(g);
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           const Stmt* bs {
               k.blkc({k.decl(v, nullptr), k.es(k.call(g, {place(k, v, N(k, "1"), N(k, "0")), N(k, "3")}))})};
           f->body = bs;
           k.done(f);
           // the scope exit's rows: n's value and flag permissions, the array leaf's, the liveness: the array leaf's value permission was taken by the callee and ends the path
           k.fail_only("lifetime", bs->end_loc, 5U, 2U);
       });
    // void fill(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) { }; main: v zero-filled; fill(&v[1].a[0], 3); v[1].a[2] = 4;
    lc("e1e2e_sarr_cells_given_and_returned",
       "the cells v[1].a[0..3) given to a callee that hands them back: the caller writes v[1].a[2] after the call",
       [=](Kit& k)
       {
           Function* g {k.fn("fill", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.ens(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           g->body = k.blk({});
           k.done(g);
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           f->body = k.blkc({k.decl(v, nullptr), k.es(k.call(g, {place(k, v, N(k, "1"), N(k, "0")), N(k, "3")})),
                             k.store(place(k, v, N(k, "1"), N(k, "2")), N(k, "4"))});
           k.done(f);
       });
    // v[i].a[j] = v[i].a[j]++ : the same leaf of a named array named twice with a write, with indexes that are not constants
    lc("e1e2e_sarr_seq_distinct_cells",
       "v[0].a[0] = v[1].a[0]++: two elements are two objects: no unsequenced row fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           const Expr* inner {k.b.inc_dec_store_expr(place(k, v, N(k, "1"), N(k, "0")), true, true, k.at())};
           const Expr* outer {k.b.store_expr(place(k, v, N(k, "0"), N(k, "0")), inner, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
           k.done(f);
       });
    // void f(int i, int j) requires(0 <= i <= 1 && 0 <= j <= 2) { struct P v[2] = {0}; v[i].a[j] = v[i].n++; } : the leaf n and the cell are two objects
    lc("e1e2e_sarr_seq_cell_and_member",
       "v[i].a[j] = v[i].n++: the array member's cell and the element's own member are two objects",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
           k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "1"), k.range(k.F(f, 1), k.i32, "0", "2")));
           Var* v {arr(k, f, P, 2U, true)};
           const Expr* inner {k.b.inc_dec_store_expr(nplace(k, v, k.pv(f, 0)), true, true, k.at())};
           const Expr* outer {k.b.store_expr(place(k, v, k.pv(f, 0), k.pv(f, 1)), inner, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
           k.done(f);
       });
    // int f(void) { struct P v[2] = {0}; int i = 0; while (i < 3) inv 0 <= i <= 3 && v[1].a[2] == 0 decreases 3 - i { i++; } check(v[1].a[2] == [0|1]); return 0; }
    const auto loop = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32)};
        Var* v {arr(k, f, P, 2U, true)};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c3 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib;
        if (c3 != nullptr)
        {
            ib.push_back(c3);
        }
        ib.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                              {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))),
                               k.cl(k.eq(read(k, v, N(k, "1"), N(k, "2")), N(k, "0")))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(ib))};
        const Stmt* c1 {k.chk(k.eq(read(k, v, N(k, "1"), N(k, "2")), N(k, mode == 1 ? "1" : "0")))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(i, N(k, "0")), lp, c1, k.ret(N(k, "0"))});
        k.done(f);
        return mode == 1 ? c1 : mode == 2 ? c3 : nullptr;
    };
    lc("e1e2e_sarr_loop_reads",
       "a loop that only reads: the array's places are carried (the invariant reads v[1].a[2], the zero is kept)",
       [=](Kit& k)
       {
           loop(k, 0);
       });
    lc("e1e2e_sarr_loop_reads_wrong_fail", "the same, then check(v[1].a[2] == 1): fails at the check",
       [=](Kit& k)
       {
           k.fail("check", loop(k, 1)->loc);
       });
    lc("e1e2e_sarr_loop_body_vacuity_fail", "the same with check(false) in the loop body: it fails",
       [=](Kit& k)
       {
           k.fail("check", loop(k, 2)->loc);
       });
    // int f(void) { struct P v[2] = {0}; int i = 0; while (i < 3) inv 0 <= i <= 3 && v[1].a[2] == 0 decreases 3 - i { v[1].a[i] = 1; i++; } return 0; }
    lc("e1e2e_sarr_loop_write_loses_values_fail",
       "the loop writes v[1].a[i]: a store reaches the member's place, so v[1].a[2] == 0 is not kept (the invariant fails at the back edge, once)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32)};
           Var* v {arr(k, f, P, 2U, true)};
           Var* i {k.local(f, "i", k.i32)};
           const Clause keep {k.cl(k.eq(read(k, v, N(k, "1"), N(k, "2")), N(k, "0")))};
           const Stmt* lp {
               k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                     {k.cl(k.sub(N(k, "3"), k.v(i)))},
                     k.blkc({k.store(place(k, v, N(k, "1"), k.v(i)), N(k, "1")), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
           f->body = k.blkc({k.decl(v, nullptr), k.decl(i, N(k, "0")), lp, k.ret(N(k, "0"))});
           k.done(f);
           k.fail("invariant-preserved", keep.loc);
       });
    // v[0].a[0] = v[0].a[1]++ : two cells of one member are two objects; v[0].a[1] = v[0].a[1]++ is one object written twice
    lc("e1e2e_sarr_seq_two_cells_of_one_member",
       "v[0].a[0] = v[0].a[1]++: two cells of one member are two objects: no row fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           const Expr* inner {k.b.inc_dec_store_expr(place(k, v, N(k, "0"), N(k, "1")), true, true, k.at())};
           const Expr* outer {k.b.store_expr(place(k, v, N(k, "0"), N(k, "0")), inner, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
           k.done(f);
       });
    // slice 3e (question 378): re-pointed from a lowering failure (`unsequenced`, e1e2e_sarr_seq_same_cell_fail) to a rejection: the Sequencer keys a cell of a member of an element of a named array
    rc("e1e2e_sarr_rej_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           const Expr* inner {k.b.inc_dec_store_expr(place(k, v, N(k, "0"), N(k, "1")), true, true, k.at())};
           const Expr* outer {k.b.store_expr(place(k, v, N(k, "0"), N(k, "1")), inner, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
           k.done(f);
       });
    // v[0].a[1] = v[1].a[1]++ : the same cell index of two elements: two objects
    lc("e1e2e_sarr_seq_two_elements_same_cell_index",
       "v[0].a[1] = v[1].a[1]++: the same cell index of two elements of one array: two objects, no row fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, true)};
           const Expr* inner {k.b.inc_dec_store_expr(place(k, v, N(k, "1"), N(k, "1")), true, true, k.at())};
           const Expr* outer {k.b.store_expr(place(k, v, N(k, "0"), N(k, "1")), inner, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
           k.done(f);
       });
    // v[i].a[0] = v[j].a[0]++ with unknown indexes: the path-precise unsequenced row
    lc("e1e2e_sarr_seq_unknown_indexes_fail",
       "v[i].a[0] = v[j].a[0]++ for unknown i and j: the cells may be one object (unsequenced)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
           k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "1"), k.range(k.F(f, 1), k.i32, "0", "1")));
           Var* v {arr(k, f, P, 2U, true)};
           const Expr* inner {k.b.inc_dec_store_expr(place(k, v, k.pv(f, 1), N(k, "0")), true, true, k.at())};
           const Expr* outer {k.b.store_expr(place(k, v, k.pv(f, 0), N(k, "0")), inner, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(outer)});
           k.done(f);
           k.fail_only("unsequenced", outer->loc, 1U, 0U);
       });
}

// ---- decision 0064 items 7, 8 and 13 (Part E, slice 3c-2): a RANGE over structures whose element has an array member of scalars: `p[i].a[j]` in code, `p[e].a[e2]` in a contract, calls, loops ----
void define_part_e_struct_array_leaf_range_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // struct P { int n; int a[3]; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    auto RO = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    // the place base[i].a[j] and its read
    auto cplace = [](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.member_addr(k.b.elem_addr(base, k.num(k.i32, i), k.at()), 1, k.at()), k.num(k.i32, j),
                             k.at());
    };
    auto cread = [=](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.deref(cplace(k, base, i, j), k.at());
    };
    // int f(struct P *p, int n) requires(<range>(p, n) && n > <lo>) ensures(<range>(p, n) && r == <want>) { p[<i>].a[<j>] = 5; return p[<i>].a[<j>]; }
    struct WR
    {
        const Stmt* store;
        const Stmt* ret;
    };
    const auto write_read = [=](Kit& k, bool owns, const char* lo, const char* want, const char* idx_i,
                                const char* j) -> WR
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(owns ? RO(k, f, 0) : RR(k, f, 0), k.gt(fx(k, f, 1), N(k, lo))));
        k.ens(f, k.land(owns ? RO(k, f, 0) : RR(k, f, 0), k.eq(k.R(f), N(k, want))));
        const Stmt* st {k.store(cplace(k, k.pv(f, 0), idx_i, j), N(k, "5"))};
        const Stmt* ret {k.ret(k.deref(cplace(k, k.pv(f, 0), idx_i, j)))};
        f->body = k.blkc({st, ret});
        k.done(f);
        return WR {st, ret};
    };
    lc("e1e2e_srange_code_rw",
       "requires owns_range(p, n) && n > 1: p[1].a[2] written and read back through the range (both index rows, the cell's rows), ensures r == 5",
       [=](Kit& k)
       {
           write_read(k, true, "1", "5", "1", "2");
       });
    lc("e1e2e_srange_code_wrong_fail", "the same ensuring r == 6: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", write_read(k, true, "1", "6", "1", "2").ret->loc);
       });
    lc("e1e2e_srange_beyond_the_count_fail",
       "p[2].a[0] with n > 1 only: the element is outside the range: the bound row and the cell's permission fail at the store",
       [=](Kit& k)
       {
           const WR w {write_read(k, true, "1", "5", "2", "0")};
           k.fail("out-of-bounds", w.store->loc, 2U);
           k.fail("permission", w.store->loc);
       });
    lc("e1e2e_srange_second_index_high_fail",
       "p[1].a[3]: the second index is outside the member's own length: out-of-bounds at the store",
       [=](Kit& k)
       {
           const WR w {write_read(k, true, "1", "5", "1", "3")};
           // (the failed bound row is assumed after it: the cell's permission row is then on an infeasible path)
           k.fail_only("out-of-bounds", w.store->loc, 2U, 1U);
       });
    lc("e1e2e_srange_write_under_reads_fail", "reads_range(p, n): p[1].a[2] written: the write row (permission)",
       [=](Kit& k)
       {
           k.fail("permission", write_read(k, false, "1", "5", "1", "2").store->loc);
       });
    // int g(struct P *p, int n) requires(<range>(p, n) && n > <lo> && p[<ri>].a[<rj>] == 5) ensures(<range>(p, n) && r == 5) { return p[1].a[2]; }
    const auto read_req = [=](Kit& k, bool owns, const char* lo, const char* ri, const char* rj) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const Expr* rd {cread(k, fx(k, g, 0), ri, rj)};
        k.req(g, k.land(k.land(owns ? RO(k, g, 0) : RR(k, g, 0), k.gt(fx(k, g, 1), N(k, lo))), k.eq(rd, N(k, "5"))));
        k.ens(g, k.land(owns ? RO(k, g, 0) : RR(k, g, 0), k.eq(k.R(g), N(k, "5"))));
        g->body = k.blkc({k.ret(k.deref(cplace(k, k.pv(g, 0), "1", "2")))});
        k.done(g);
        return rd;
    };
    lc("e1e2e_srange_contract_read_reads",
       "requires reads_range(p, n) && n > 1 && p[1].a[2] == 5: the read is framed (the slot is in the range, the column is in the member's cells)",
       [=](Kit& k)
       {
           read_req(k, false, "1", "1", "2");
       });
    lc("e1e2e_srange_contract_read_owns", "the same over owns_range(p, n)",
       [=](Kit& k)
       {
           read_req(k, true, "1", "1", "2");
       });
    lc("e1e2e_srange_contract_read_outside_fail",
       "n > 0 only: p[1].a[2] may lie outside the range: unowned (permission)",
       [=](Kit& k)
       {
           k.fail("permission", read_req(k, true, "0", "1", "2")->loc);
       });
    lc("e1e2e_srange_contract_read_second_index_fail",
       "p[1].a[3]: no cell of the member is that place: unowned (permission)",
       [=](Kit& k)
       {
           k.fail("permission", read_req(k, true, "1", "1", "3")->loc);
       });
    // int g(struct P *p, int n) requires(<range>(p, n) && n > 1) ensures(<range>(p, n) && r == 7) { [p[1].a[2] = 7; return p[1].a[2];] | [return 7;] }
    const auto mk_g = [=](Kit& k, StructType* P, bool owns) -> Function*
    {
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(g, k.land(owns ? RO(k, g, 0) : RR(k, g, 0), k.gt(fx(k, g, 1), N(k, "1"))));
        k.ens(g, k.land(owns ? RO(k, g, 0) : RR(k, g, 0), k.eq(k.R(g), N(k, "7"))));
        if (owns)
        {
            g->body = k.blkc(
                {k.store(cplace(k, k.pv(g, 0), "1", "2"), N(k, "7")), k.ret(k.deref(cplace(k, k.pv(g, 0), "1", "2")))});
        }
        else
        {
            g->body = k.blkc({k.ret(N(k, "7"))});
        }
        k.done(g);
        return g;
    };
    // int f(struct P *p, int n) requires(<range>(p, n) && n > 1) ensures(<range>(p, n) && r == <want>) { return g(p, n); }
    const auto forward = [=](Kit& k, const char* want, bool owns_in, bool owns_callee) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, owns_callee)};
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
    lc("e1e2e_srange_call_forward",
       "f(p, n) forwards its owns_range(p, n) over struct P (an array member) to g, whose contract writes p[1].a[2]: the call rows per leaf path (the array leaf over two bound variables), r == 7 reaches f's",
       [=](Kit& k)
       {
           forward(k, "7", true, true);
       });
    lc("e1e2e_srange_call_forward_wrong_fail", "the same, f's ensures claims r == 8: the postcondition at f's return",
       [=](Kit& k)
       {
           k.fail("postcondition", forward(k, "8", true, true));
       });
    lc("e1e2e_srange_call_reads_callee_reads",
       "f holds reads_range(p, n) and calls g, which reads it: the amounts are fractions of the caller's own",
       [=](Kit& k)
       {
           forward(k, "7", false, false);
       });
    lc("e1e2e_srange_call_owns_needed_reads_held_fail",
       "f holds reads_range(p, n) and calls g, which needs to own it: a marked row at write permission fails (permission); the path ends there",
       [=](Kit& k)
       {
           k.fail_only("permission", forward(k, "7", false, true), 12U, 5U);
       });
    // void f(void) { struct P v[3] = {0} or no initializer; int r = g(&v[0], <n>); check(r == 7); [check(v[1].a[2] == 0);] }
    const auto local_call = [=](Kit& k, unsigned len, const char* n, bool zero, bool owns, bool kept) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, owns)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), len), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* c {k.call(g, {first, N(k, n)})};
        Var* r {k.local(f, "r", k.i32)};
        const Stmt* chk {k.chk(k.eq(k.v(r), N(k, "7")))};
        std::vector<const Stmt*> body {k.decl(v, nullptr), k.decl(r, c), chk};
        if (kept)
        {
            body.push_back(k.chk(k.eq(k.deref(cplace(k, k.addr(v), "1", "2")), N(k, "0"))));
        }
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_srange_call_local_array",
       "struct P v[3] = {0}; g(&v[0], 3): the range of the whole local array, then r == 7 is known",
       [=](Kit& k)
       {
           local_call(k, 3U, "3", true, true, false);
       });
    lc("e1e2e_srange_call_local_array_reads_keeps_value",
       "struct P v[3] = {0}; a reading callee: the caller keeps its permission and the value v[1].a[2] == 0",
       [=](Kit& k)
       {
           local_call(k, 3U, "3", true, false, true);
       });
    lc("e1e2e_srange_call_local_array_too_long_fail",
       "struct P v[3] = {0}; g(&v[0], 4): the shape row of the range fails (permission at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", local_call(k, 3U, "4", true, true, false), 12U, 0U);
       });
    lc("e1e2e_srange_call_local_array_unset_fail",
       "struct P v[3] with no initializer; g(&v[0], 3): no element has a value: the flag row of each leaf fails (uninitialized: n and a)",
       [=](Kit& k)
       {
           k.fail("uninitialized", local_call(k, 3U, "3", false, true, false), 2U);
       });
    lc("e1e2e_srange_call_local_array_wrong_value_fail",
       "struct P v[3] = {0}; an owning callee may have changed v[1].a[2]: check(v[1].a[2] == 0) fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* g {mk_g(k, P, true)};
           Function* f {k.fn("f", k.vd)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
           f->mark_address_taken(v);
           v->zero_filled = true;
           Var* r {k.local(f, "r", k.i32)};
           const Stmt* chk {k.chk(k.eq(k.deref(cplace(k, k.addr(v), "1", "2")), N(k, "0")))};
           f->body = k.blkc({k.decl(v, nullptr),
                             k.decl(r, k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "3")})), chk});
           k.done(f);
           k.fail("check", chk->loc);
       });
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == <want>) { int i = 0; while (i < 3) inv 0 <= i <= 3 decreases 3 - i { p[0].a[i] = 1; i++; } p[1].a[2] = 4; return p[1].a[2]; }
    const auto loop = [=](Kit& k, const char* want, bool owns) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(owns ? RO(k, f, 0) : RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(owns ? RO(k, f, 0) : RR(k, f, 0), k.eq(k.R(f), N(k, want))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* wp {
            k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()), 1, k.at()), k.v(i), k.at())};
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3"))))},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))},
                              k.blkc({k.store(wp, N(k, "1")), k.asg(i, k.add(k.v(i), N(k, "1")))}))};
        const Stmt* ret {k.ret(k.deref(cplace(k, k.pv(f, 0), "1", "2")))};
        f->body = k.blkc({k.decl(i, N(k, "0")), lp, k.store(cplace(k, k.pv(f, 0), "1", "2"), N(k, "4")), ret});
        k.done(f);
        return ret;
    };
    lc("e1e2e_srange_loop_keeps_the_range",
       "a loop that writes p[0].a[i] under owns_range(p, n): the range's places (the array leaf over two bound variables) are carried; after it p[1].a[2] = 4 is written and read",
       [=](Kit& k)
       {
           loop(k, "4", true);
       });
    lc("e1e2e_srange_loop_wrong_fail", "the same ensuring r == 5: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", loop(k, "5", true)->loc);
       });
    // ---- pairing (record item 7) and sequencing (item 11) of a range over such structures with a cell ----
    // void c2(struct P *p, int n, int *q) requires(owns_range(p, n) && owns(*q) && n > 0) ensures(owns_range(p, n) && owns(*q)) { }
    // void f(void) { struct P v[3] = {0}; int c = 0; c2(&v[0], <n>, <q>); }   q: &c (qelem < 0), &v[qelem].a[qj] or &v[qelem].n (qj < 0)
    const auto pair_cell = [=](Kit& k, const char* n, int qelem, int qj) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c {k.fn("c2", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
        k.req(c, k.land(k.land(RO(k, c, 0), k.b.atom(AtomKind::Owns, fx(k, c, 2), 1, k.at())),
                        k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c, k.land(RO(k, c, 0), k.b.atom(AtomKind::Owns, fx(k, c, 2), 1, k.at())));
        c->body = k.blkc({});
        k.done(c);
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* cc {k.cell(f, "c", k.i32)};
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const std::string es {std::to_string(qelem)};
        const std::string js {std::to_string(qj)};
        const Expr* q {qelem < 0 ? k.addr(cc)
                       : qj < 0  ? k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, es.c_str()), k.at()), 0, k.at())
                                 : cplace(k, k.addr(v), es.c_str(), js.c_str())};
        const Expr* call {k.call(c, {first, N(k, n), q})};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(cc, N(k, "0")), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2e_srange_pair_cell_outside",
       "c2(&v[0], 2, &c) with c a separate local, owns_range(p, n) && owns(*q): the cell is outside the range's cells: every row passes",
       [=](Kit& k)
       {
           pair_cell(k, "2", -1, 0);
       });
    lc("e1e2e_srange_pair_cell_in_a_later_element",
       "c2(&v[0], 2, &v[2].a[1]): the cell is a cell of element 2's member, outside the range [0, 2): the row of the array leaf passes",
       [=](Kit& k)
       {
           pair_cell(k, "2", 2, 1);
       });
    lc("e1e2e_srange_pair_array_cell_alias_fail",
       "c2(&v[0], 3, &v[1].a[2]): the cell is a cell of the array leaf of element 1, which the range holds: the row of the array leaf fails (permission), the row of n passes",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, "3", 1, 2), 2U, 1U, "which may be one object");
       });
    lc("e1e2e_srange_pair_member_cell_alias_fail",
       "c2(&v[0], 3, &v[1].n): the cell is the member n of element 1: the row of n fails (permission), the array leaf's passes",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, "3", 1, -1), 2U, 0U, "which may be one object");
       });
    // void put(struct P *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n)) { }; void f(void) { struct P v[3] = {0}; put(&v[0], <n>) + v[1].a[2]; }
    const auto seq_read = [=](Kit& k, const char* n) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {k.fn("h", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(h, k.land(RO(k, h, 0), k.gt(fx(k, h, 1), N(k, "0"))));
        k.ens(h, k.land(RO(k, h, 0), k.eq(k.R(h), N(k, "1"))));
        h->body = k.blkc({k.store(cplace(k, k.pv(h, 0), "0", "0"), N(k, "1")), k.ret(N(k, "1"))});
        k.done(h);
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* rd {k.deref(cplace(k, k.addr(v), "1", "2"))};
        const Expr* c {k.call(h, {first, N(k, n)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c, rd))});
        k.done(f);
        return rd->loc;
    };
    lc("e1e2e_srange_seq_call_and_read_disjoint",
       "h(&v[0], 1) == v[1].a[2]: the call's range is [0, 1), the read is element 1's member: the rows pass",
       [=](Kit& k)
       {
           seq_read(k, "1");
       });
    lc("e1e2e_srange_seq_call_and_read_overlap_fail",
       "h(&v[0], 2) == v[1].a[2]: the read is a cell the call's range holds, in an order C leaves unspecified (unsequenced at the read)",
       [=](Kit& k)
       {
           // two rows (the leaves n and a): the row of n passes, the array leaf's fails
           k.fail_only("unsequenced", seq_read(k, "2"), 2U, 1U);
       });
    // void f(struct P *p, int n) requires(reads_range(p, n) && n > 1 && p[0].a[1] == 3 && p[1].a[0] == 4 && p[1].a[1] == 5 && p[0].a[1] == 3) { }
    lc("e1e2e_srange_contract_reads_keys",
       "four reads of three distinct cells of members of elements of a range under reads_range: the element index and the member index are each part of the key: three ghosts, none unframed",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           auto rd = [&](const char* i, const char* j)
           {
               return cread(k, fx(k, f, 0), i, j);
           };
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))),
                           k.land(k.land(k.eq(rd("0", "1"), N(k, "3")), k.eq(rd("1", "0"), N(k, "4"))),
                                  k.land(k.eq(rd("1", "1"), N(k, "5")), k.eq(rd("0", "1"), N(k, "3"))))));
           f->body = k.blk({});
           k.done(f);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   std::set<std::string> out;
                   for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
                   {
                       size_t end {at + 4U};
                       while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
                       {
                           ++end;
                       }
                       out.insert(vpr.substr(at, end - at));
                   }
                   return out.size() == 3U
                              ? ""
                              : "the ghosts of the four reads: expected 3 (one per distinct cell), found " +
                                    std::to_string(out.size());
               });
       });
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && p[1].a[2] == old(p[1].a[2]) + <d>) { p[1].a[2] = p[1].a[2] + 1; return 0; }
    const auto old_read = [=](Kit& k, const char* d) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))),
                        k.lt(cread(k, fx(k, f, 0), "1", "2"), N(k, "1000"))));
        k.ens(f, k.land(RO(k, f, 0),
                        k.eq(cread(k, fx(k, f, 0), "1", "2"), k.add(k.old(cread(k, fx(k, f, 0), "1", "2")), N(k, d)))));
        const Stmt* ret {k.ret(N(k, "0"))};
        f->body = k.blkc(
            {k.store(cplace(k, k.pv(f, 0), "1", "2"), k.add(k.deref(cplace(k, k.pv(f, 0), "1", "2")), N(k, "1"))),
             ret});
        k.done(f);
        return ret;
    };
    lc("e1e2e_srange_old_read",
       "ensures p[1].a[2] == old(p[1].a[2]) + 1 after p[1].a[2] = p[1].a[2] + 1 under owns_range: the old read is the entry state's value (question 346's limit does not apply to a parameter's range)",
       [=](Kit& k)
       {
           old_read(k, "1");
       });
    lc("e1e2e_srange_old_read_wrong_fail", "the same with + 2: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", old_read(k, "2")->loc);
       });
    // int f(struct P *p, int n) requires(<range>(p, n) && n > 1 && p[<ri>].a[<rj>] != 0) ensures(<range>(p, n) && r == 10 / p[<ei>].a[<ej>]) { return 10 / p[<ri>].a[<rj>]; }
    struct TieLocs
    {
        const Expr* ens_div;
        SrcLoc ret;
        const Expr* req_read;
    };
    const auto tie = [=](Kit& k, bool with_atom, bool reads, const char* ri, const char* rj, const char* ei,
                         const char* ej) -> TieLocs
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        auto atom = [&]() -> const Expr*
        {
            return reads ? RR(k, f, 0) : RO(k, f, 0);
        };
        const Expr* rd1 {cread(k, fx(k, f, 0), ri, rj)};
        const Expr* d {k.div(N(k, "10"), cread(k, fx(k, f, 0), ei, ej))};
        const Expr* pre {k.land(k.gt(fx(k, f, 1), N(k, "1")), k.ne(rd1, N(k, "0")))};
        k.req(f, with_atom ? k.land(atom(), pre) : pre);
        k.ens(f, with_atom ? k.land(atom(), k.eq(k.R(f), d)) : k.eq(k.R(f), d));
        const Stmt* rets {k.ret(k.div(N(k, "10"), k.deref(cplace(k, k.pv(f, 0), ri, rj))))};
        f->body = k.blk({rets});
        k.done(f);
        return TieLocs {d, rets->loc, rd1};
    };
    lc("e1e2e_srange_contract_read_tied",
       "requires reads_range(p, n) && n > 1 && p[1].a[0] != 0, ensures r == 10 / p[1].a[0]: tied across the two states under reads_range (question 376), so the division at the ensures is by a nonzero",
       [=](Kit& k)
       {
           tie(k, true, true, "1", "0", "1", "0");
       });
    lc("e1e2e_srange_contract_read_owns_fail",
       "the same under owns_range: the function may write the cell, no tie (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail("division-by-zero", tie(k, true, false, "1", "0", "1", "0").ens_div->loc);
       });
    lc("e1e2e_srange_contract_read_other_cell_fail",
       "requires p[1].a[0] != 0, ensures dividing by p[1].a[1]: another cell of the member has its own ghost (division-by-zero at the ensures, and the body's postcondition)",
       [=](Kit& k)
       {
           const TieLocs t {tie(k, true, true, "1", "0", "1", "1")};
           k.fail("division-by-zero", t.ens_div->loc);
           k.fail("postcondition", t.ret);
       });
    lc("e1e2e_srange_contract_read_other_element_fail",
       "requires p[0].a[1] != 0, ensures dividing by p[1].a[1]: another element, the same cell index (division-by-zero at the ensures, and the body's postcondition)",
       [=](Kit& k)
       {
           const TieLocs t {tie(k, true, true, "0", "1", "1", "1")};
           k.fail("division-by-zero", t.ens_div->loc);
           k.fail("postcondition", t.ret);
       });
    // struct Q { int a[2]; int b[2]; }: void f(struct Q *p, int n) requires(reads_range(p, n) && n > 1 && p[0].a[1] == 1 && p[0].b[1] == 1) { }
    lc("e1e2e_srange_contract_reads_keys_members",
       "p[0].a[1] and p[0].b[1] under reads_range(p, n): two members of one element are two keys (the member path is part of the key): two ghosts",
       [=](Kit& k)
       {
           StructType* Q {k.strukt("Q", {{"a", k.prog.array_of(k.i32, 2U)}, {"b", k.prog.array_of(k.i32, 2U)}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           auto rd = [&](int member)
           {
               return k.deref(k.b.elem_addr(
                   k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), member, k.at()), N(k, "1"), k.at()));
           };
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))),
                           k.land(k.eq(rd(0), N(k, "1")), k.eq(rd(1), N(k, "1")))));
           f->body = k.blk({});
           k.done(f);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   std::set<std::string> out;
                   for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
                   {
                       size_t end {at + 4U};
                       while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
                       {
                           ++end;
                       }
                       out.insert(vpr.substr(at, end - at));
                   }
                   return out.size() == 2U ? ""
                                           : "the ghosts of the two reads: expected 2 (one per member), found " +
                                                 std::to_string(out.size());
               });
       });
}

// ---- coordinator's addition (slice 3b-5): a matrix member reached through a CHAIN of member steps: struct V { struct U u; int k; }, struct U { int n; int m[2][3]; } ----
void define_part_e_nested_matrix_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto U = [](Kit& k) -> StructType*
    {
        return k.strukt("U", {{"n", k.i32}, {"m", e1e2e_matrix(k)}});
    };
    auto V = [=](Kit& k) -> StructType*
    {
        return k.strukt("V", {{"u", k.st(U(k))}, {"k", k.i32}});
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // the cell base->u.m[i][j]
    auto cell = [](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(k.mem(base, 0), 1), k.num(k.i32, i), k.at()), k.num(k.i32, j), k.at());
    };
    // void put(struct V *p) requires(<owns(*p) | owns(p->u)>) ensures(<same>) { p->u.m[1][<j>] = 5; }
    const auto make_put = [=](Kit& k, StructType* S, bool whole, const char* j) -> std::pair<Function*, const Stmt*>
    {
        Function* f {k.fn("put", k.vd, {{"p", k.ptr(k.st(S))}})};
        const Expr* a1 {whole ? k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at())
                              : k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 0), 0, k.at())};
        const Expr* a2 {whole ? k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at())
                              : k.b.atom(AtomKind::Owns, k.mem(FP(k, f, 0), 0), 0, k.at())};
        k.req(f, a1);
        k.ens(f, a2);
        const Stmt* st {k.store(cell(k, k.pv(f, 0), "1", j), N(k, "5"))};
        f->body = k.blk({st});
        k.done(f);
        return {f, st};
    };
    // void use(void) { struct V w; zero fill of w.u.m; w.u.n = 1; w.k = 2; put(&w); w.u.m[0][0] = 1; check(w.u.m[0][0] == <want> [&& w.k == 2]); }
    const auto use = [=](Kit& k, bool whole, const char* want, bool check_k) -> const Stmt*
    {
        StructType* S {V(k)};
        Function* put {make_put(k, S, whole, "2").first};
        Function* f {k.fn("use", k.vd)};
        Var* w {k.scell(f, "w", S)};
        const Expr* chk_e {k.eq(k.deref(cell(k, k.addr(w), "0", "0")), N(k, want))};
        if (check_k)
        {
            chk_e = k.land(chk_e, k.eq(k.deref(k.mem(k.addr(w), 1)), N(k, "2")));
        }
        const Stmt* c {k.chk(chk_e)};
        f->body = k.blkc({k.decl(w, nullptr), k.b.array_zero(k.mem(k.mem(k.addr(w), 0), 1), k.at()),
                          k.store(k.mem(k.mem(k.addr(w), 0), 0), N(k, "1")), k.store(k.mem(k.addr(w), 1), N(k, "2")),
                          k.es(k.call(put, {k.addr(w)})), k.store(cell(k, k.addr(w), "0", "0"), N(k, "1")), c});
        k.done(f);
        return c;
    };
    lc("e1e2e_nested_matrix_owns_whole",
       "put(struct V *p) with owns(*p): the callee writes p->u.m[1][2], the caller writes w.u.m[0][0] after the call and reads it back",
       [=](Kit& k)
       {
           use(k, true, "1", false);
       });
    lc("e1e2e_nested_matrix_owns_member_keeps_k",
       "put with owns(p->u): the callee holds only the member u; the caller's w.k == 2 survives the call and w.u.m[0][0] is read back",
       [=](Kit& k)
       {
           use(k, false, "1", true);
       });
    lc("e1e2e_nested_matrix_wrong_fail", "the same checking w.u.m[0][0] == 2 after writing 1: the check fails",
       [=](Kit& k)
       {
           k.fail("check", use(k, true, "2", false)->loc);
       });
    lc("e1e2e_nested_matrix_second_index_high_fail",
       "the callee writes p->u.m[1][3]: the second index is outside the row's own length: out-of-bounds at the store",
       [=](Kit& k)
       {
           StructType* S {V(k)};
           k.fail_only("out-of-bounds", make_put(k, S, true, "3").second->loc, 2U, 1U);
       });
    // int main-like: struct V w (zero-filled member, n and k set); put(&w) + w.<place>;   put holds owns(*p) or owns(p->u)
    const auto seq = [=](Kit& k, bool whole, int place) -> const Expr*
    {
        StructType* S {V(k)};
        Function* put {k.fn("put", k.i32, {{"p", k.ptr(k.st(S))}})};
        const Expr* a {whole ? k.b.atom(AtomKind::Owns, FP(k, put, 0), 0, k.at())
                             : k.b.atom(AtomKind::Owns, k.mem(FP(k, put, 0), 0), 0, k.at())};
        const Expr* b {whole ? k.b.atom(AtomKind::Owns, FP(k, put, 0), 0, k.at())
                             : k.b.atom(AtomKind::Owns, k.mem(FP(k, put, 0), 0), 0, k.at())};
        k.req(put, a);
        k.ens(put, k.land(b, k.eq(k.R(put), N(k, "1"))));
        put->body = k.blk({k.ret(N(k, "1"))});
        k.done(put);
        Function* f {k.fn("f", k.vd)};
        Var* w {k.scell(f, "w", S)};
        Var* z {k.local(f, "z", k.i32)};
        const Expr* rd {place == 0 ? k.deref(cell(k, k.addr(w), "1", "2")) : k.deref(k.mem(k.addr(w), 1))};
        f->body = k.blkc({k.decl(w, nullptr), k.b.array_zero(k.mem(k.mem(k.addr(w), 0), 1), k.at()),
                          k.store(k.mem(k.mem(k.addr(w), 0), 0), N(k, "1")), k.store(k.mem(k.addr(w), 1), N(k, "2")),
                          k.decl(z, k.add(k.call(put, {k.addr(w)}), rd))});
        k.done(f);
        return rd;
    };
    lc("e1e2e_nested_matrix_seq_disjoint",
       "put(&w) holding owns(p->u) beside w.k: the call holds the member u, w.k is another object: no row fails",
       [=](Kit& k)
       {
           seq(k, false, 1);
       });
    lc("e1e2e_nested_matrix_seq_overlap_fail",
       "put(&w) holding owns(p->u) beside w.u.m[1][2]: the call holds every cell of the nested matrix (unsequenced)",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq(k, false, 0)->loc, 2U, 1U);
       });
    lc("e1e2e_nested_matrix_seq_whole_overlap_fail", "put(&w) holding owns(*p) beside w.u.m[1][2]: unsequenced",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq(k, true, 0)->loc, 3U, 2U);
       });
    // struct W { struct U u; struct U u2; }: void f(struct W *p) requires(reads(*p) && p->u.m[0][1] == 3 && p->u2.m[0][1] == 4 && p->u.m[1][0] == 5 && p->u.m[0][1] == 3) { }
    lc("e1e2e_nested_matrix_contract_reads_keys",
       "reads of p->u.m[0][1], p->u2.m[0][1] and p->u.m[1][0] (and p->u.m[0][1] again) under reads(*p): the chain u / u2 is part of the key: three ghosts, none unframed",
       [=](Kit& k)
       {
           StructType* UU {U(k)};
           StructType* S {k.strukt("W", {{"u", k.st(UU)}, {"u2", k.st(UU)}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
           auto rd = [&](int member, const char* i, const char* j)
           {
               return k.deref(
                   k.b.elem_addr(k.b.elem_addr(k.mem(k.mem(k.F(f, 0), member), 1), N(k, i), k.at()), N(k, j), k.at()));
           };
           k.req(f, k.land(k.b.atom(AtomKind::Reads, FP(k, f, 0), 0, k.at()),
                           k.land(k.land(k.eq(rd(0, "0", "1"), N(k, "3")), k.eq(rd(1, "0", "1"), N(k, "4"))),
                                  k.land(k.eq(rd(0, "1", "0"), N(k, "5")), k.eq(rd(0, "0", "1"), N(k, "3"))))));
           f->body = k.blk({});
           k.done(f);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   std::set<std::string> out;
                   for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
                   {
                       size_t end {at + 4U};
                       while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
                       {
                           ++end;
                       }
                       out.insert(vpr.substr(at, end - at));
                   }
                   return out.size() == 3U
                              ? ""
                              : "the ghosts of the four reads: expected 3 (one per distinct cell), found " +
                                    std::to_string(out.size());
               });
       });
    // int f(struct V *p) requires(owns(*p) && p->u.m[1][0] == 4) ensures(owns(*p) && r == p->u.m[1][0]) { return p->u.m[1][0]; }: the contract read through the chain
    lc("e1e2e_nested_matrix_contract_read",
       "requires owns(*p) && p->u.m[1][0] == 4, ensures r == p->u.m[1][0]: the read through the chain of members is framed by the structure atom's array part",
       [=](Kit& k)
       {
           StructType* S {V(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(S))}})};
           k.req(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()),
                           k.eq(k.deref(cell(k, k.F(f, 0), "1", "0")), N(k, "4"))));
           k.ens(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()),
                           k.eq(k.R(f), k.deref(cell(k, k.F(f, 0), "1", "0")))));
           f->body = k.blk({k.ret(k.deref(cell(k, k.pv(f, 0), "1", "0")))});
           k.done(f);
       });
    lc("e1e2e_nested_matrix_contract_read_wrong_fail",
       "the same ensuring r == p->u.m[1][1]: the postcondition fails at the return",
       [=](Kit& k)
       {
           StructType* S {V(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(S))}})};
           k.req(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()),
                           k.eq(k.deref(cell(k, k.F(f, 0), "1", "0")), N(k, "4"))));
           k.ens(f, k.land(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()),
                           k.eq(k.R(f), k.deref(cell(k, k.F(f, 0), "1", "1")))));
           const Stmt* ret {k.ret(k.deref(cell(k, k.pv(f, 0), "1", "0")))};
           f->body = k.blk({ret});
           k.done(f);
           k.fail("postcondition", ret->loc);
       });
}

// ---- decision 0064 items 9 and 13 (Part E, slice 3c-3): a GLOBAL array of structures whose element has an array member of scalars: program_start's row group, main's reads and writes, a callee that owns it ----
void define_part_e_struct_array_leaf_global_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    // struct P { int n; int a[3]; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    // struct P g[2] = {{7, {1, 2, 3}}, {9, {4}}};   (`unknown`: the linkage and escape case, every cell unknown)
    const auto gs_global = [=](Kit& k, StructType* P, bool unknown, const Function* owner = nullptr) -> Global*
    {
        Global* g {k.prog.add_global("g", k.prog.array_of(k.st(P), 2U), k.at(), owner)};
        const std::vector<std::pair<const char*, std::vector<const char*>>> els {{"7", {"1", "2", "3"}}, {"9", {"4"}}};
        for (const auto& [nv, cells] : els)
        {
            GlobalLeaf el;
            el.kind = GlobalInit::Value;
            el.loc = k.at();
            GlobalLeaf ln;
            ln.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
            ln.init = k.num(k.i32, nv);
            ln.value = nv;
            ln.loc = k.at();
            el.leaves.push_back(ln);
            GlobalLeaf la;
            la.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
            la.rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
            la.loc = k.at();
            for (const char* v : cells)
            {
                GlobalLeaf c;
                c.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
                c.value = v;
                c.init = k.num(k.i32, v);
                c.loc = k.at();
                la.elems.push_back(c);
            }
            el.leaves.push_back(la);
            g->elems.push_back(el);
        }
        g->rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
        return g;
    };
    auto gplace = [](Kit& k, const Global* g, const char* i, const char* j)
    {
        return k.b.elem_addr(
            k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), k.num(k.i32, i), k.at()), 1, k.at()),
            k.num(k.i32, j), k.at());
    };
    auto gn = [](Kit& k, const Global* g, const char* i)
    {
        return k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), k.num(k.i32, i), k.at()), 0, k.at());
    };
    // int main(void) { check(g[0].n == 7 && g[0].a[1] == 2 && g[1].n == 9 && g[1].a[0] == 4 && g[1].a[2] == <want>); [g[1].a[2] = 5; check(g[1].a[2] == 5 && g[0].a[0] == 1);] return 0; }
    const auto main_rw = [=](Kit& k, const char* want, bool write) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Global* g {gs_global(k, P, false)};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c1 {k.chk(k.land(
            k.land(k.eq(k.deref(gn(k, g, "0")), N(k, "7")), k.eq(k.deref(gplace(k, g, "0", "1")), N(k, "2"))),
            k.land(k.land(k.eq(k.deref(gn(k, g, "1")), N(k, "9")), k.eq(k.deref(gplace(k, g, "1", "0")), N(k, "4"))),
                   k.eq(k.deref(gplace(k, g, "1", "2")), N(k, want)))))};
        std::vector<const Stmt*> body {c1};
        if (write)
        {
            body.push_back(k.store(gplace(k, g, "1", "2"), N(k, "5")));
            body.push_back(k.chk(k.land(k.eq(k.deref(gplace(k, g, "1", "2")), N(k, "5")),
                                        k.eq(k.deref(gplace(k, g, "0", "0")), N(k, "1")))));
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return c1;
    };
    lc("e1e2e_gsarr_main_read",
       "struct P g[2] = {{7, {1, 2, 3}}, {9, {4}}}: main reads the listed cells of the member of each element and the zero rest",
       [=](Kit& k)
       {
           main_rw(k, "0", false);
       });
    lc("e1e2e_gsarr_main_read_wrong_fail", "the same: the check that g[1].a[2] is 1 fails (the zero rest)",
       [=](Kit& k)
       {
           k.fail("check", main_rw(k, "1", false)->loc);
       });
    lc("e1e2e_gsarr_main_write",
       "g[1].a[2] = 5 in main: the member's cells are held with write permission from program_start, the other cells keep their values",
       [=](Kit& k)
       {
           main_rw(k, "0", true);
       });
    lc("e1e2e_gsarr_main_vacuity_fail",
       "program_start's facts for the member are satisfiable: a check(false) after the reads is reached and fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Global* g {gs_global(k, P, false)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.no())};
           m->body = k.blkc({k.chk(k.eq(k.deref(gplace(k, g, "0", "1")), N(k, "2"))), c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2e_gsarr_unknown_cells_fail", "an unknown initial value (linkage): g[0].a[0] == 1 is not provable (check)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Global* g {gs_global(k, P, true)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq(k.deref(gplace(k, g, "0", "0")), N(k, "1")))};
           m->body = k.blkc({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    // void put(void) requires(owns(g)) ensures(owns(g)) { g[1].a[2] = 7; }   int main() { put(); g[0].a[0] = 9; check(g[0].n == 7 [&& g[1].a[2] == 7]); }
    const auto call = [=](Kit& k, const char* want_n, bool want_cell) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Global* g {gs_global(k, P, false)};
        auto atom = [&]()
        {
            return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        Function* put {k.fn("put", k.vd)};
        k.req(put, atom());
        k.ens(put, atom());
        put->body = k.blk({k.store(gplace(k, g, "1", "2"), N(k, "7"))});
        k.done(put);
        Function* m {k.fn("main", k.i32)};
        (void)want_n;
        const Stmt* c {want_cell ? k.chk(k.eq(k.deref(gplace(k, g, "1", "2")), N(k, "7"))) : k.chk(k.yes())};
        m->body = k.blkc({k.es(k.call(put, {})), k.store(gplace(k, g, "0", "0"), N(k, "9")), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_gsarr_call_owns",
       "main calls put() with owns(g): the callee writes g[1].a[2]; main writes g[0].a[0] after the call (the member's cells come back)",
       [=](Kit& k)
       {
           call(k, "7", false);
       });
    lc("e1e2e_gsarr_call_owns_value_unknown_fail",
       "owns(g) promises no value: check(g[1].a[2] == 7) after the call fails (the callee's write is not in its contract)",
       [=](Kit& k)
       {
           k.fail("check", call(k, "7", true)->loc);
       });
    // ---- the contract read g[e].a[e2] of a global array of such structures, framed by owns(g) / reads(g), tied across the two states under reads (question 376) ----
    // int f(void) requires(<atom>(g) && g[<ri>].a[<rj>] != 0) ensures(<atom>(g) && r == 10 / g[<ei>].a[<ej>]) { return 10 / g[<ri>].a[<rj>]; }   int main() { return f(); }
    const auto lsread = [=](Kit& k, bool with_atom, bool reads, const char* ri, const char* rj, const char* ei,
                            const char* ej, SrcLoc* call_loc = nullptr, SrcLoc* fn_loc = nullptr,
                            SrcLoc* ret_loc = nullptr) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Global* g {gs_global(k, P, false)};
        Function* f {k.fn("f", k.i32)};
        const AtomKind kind {reads ? AtomKind::Reads : AtomKind::Owns};
        auto atom = [&]()
        {
            return k.b.global_atom(kind, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        const Expr* rd1 {k.deref(gplace(k, g, ri, rj))};
        if (fn_loc != nullptr)
        {
            *fn_loc = rd1->loc;
        }
        const Expr* pre {k.ne(rd1, N(k, "0"))};
        const Expr* d {k.div(N(k, "10"), k.deref(gplace(k, g, ei, ej)))};
        k.req(f, with_atom ? k.land(atom(), pre) : pre);
        k.ens(f, with_atom ? k.land(atom(), k.eq(k.R(f), d)) : k.eq(k.R(f), d));
        const Stmt* rets {k.ret(k.div(N(k, "10"), k.deref(gplace(k, g, ri, rj))))};
        if (ret_loc != nullptr)
        {
            *ret_loc = rets->loc;
        }
        f->body = k.blk({rets});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(f, {})};
        if (call_loc != nullptr)
        {
            *call_loc = callx->loc;
        }
        m->body = k.blk({k.ret(callx)});
        k.done(m);
        k.prog.set_entry(m);
        return d;
    };
    lc("e1e2e_gsarr_contract_read_tied",
       "requires reads(g) && g[1].a[0] != 0, ensures r == 10 / g[1].a[0]: framed by the whole-array atom, tied across the two states under reads, so the division is by a nonzero; main's g[1].a[0] is 4 from program_start",
       [=](Kit& k)
       {
           lsread(k, true, true, "1", "0", "1", "0");
       });
    lc("e1e2e_gsarr_contract_read_owns_fail",
       "the same under owns(g): the function may write the cell, no tie, the ensures' division is not proved (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           k.fail("division-by-zero", lsread(k, true, false, "1", "0", "1", "0")->loc);
       });
    lc("e1e2e_gsarr_contract_read_other_cell_fail",
       "requires g[1].a[0] != 0 and ensures dividing by g[1].a[1] under reads: another cell of the same member has its own ghost (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           SrcLoc ret;
           const Expr* d {lsread(k, true, true, "1", "0", "1", "1", nullptr, nullptr, &ret)};
           k.fail("division-by-zero", d->loc);
           k.fail("postcondition", ret);
       });
    lc("e1e2e_gsarr_contract_read_other_element_fail",
       "requires g[0].a[1] != 0 and ensures dividing by g[1].a[1]: another element, the same cell index (division-by-zero at the ensures)",
       [=](Kit& k)
       {
           SrcLoc ret;
           const Expr* d {lsread(k, true, true, "0", "1", "1", "1", nullptr, nullptr, &ret)};
           k.fail("division-by-zero", d->loc);
           k.fail("postcondition", ret);
       });
    lc("e1e2e_gsarr_contract_read_unframed_fail",
       "the same with no atom on g: the read g[1].a[0] in the requires is held by nothing (permission at the read) and main's precondition",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc fn;
           lsread(k, false, false, "1", "0", "1", "0", &call, &fn);
           k.fail("precondition", call);
           k.fail("permission", fn);
       });
    // struct Q { int a[2]; int b[2]; } q[2]: void f(void) requires(reads(q) && q[0].a[1] == 1 && q[0].b[1] == 1 && q[1].a[1] == 1 && q[0].a[0] == 1 && q[0].a[1] == 1) { }
    lc("e1e2e_gsarr_contract_reads_keys",
       "five reads of four distinct cells of two members of two elements of a global array under reads(q): the element, the member and the cell index are each part of the key: four ghosts, none unframed",
       [=](Kit& k)
       {
           StructType* Q {k.strukt("Q", {{"a", k.prog.array_of(k.i32, 2U)}, {"b", k.prog.array_of(k.i32, 2U)}})};
           Global* g {k.prog.add_global("q", k.prog.array_of(k.st(Q), 2U), k.at(), nullptr)};
           for (int e {0}; e < 2; ++e)
           {
               GlobalLeaf el;
               el.kind = GlobalInit::Value;
               el.loc = k.at();
               for (int m {0}; m < 2; ++m)
               {
                   GlobalLeaf la;
                   la.kind = GlobalInit::Value;
                   la.rest = ArrayRest::Zero;
                   la.loc = k.at();
                   for (const char* v : {"1", "1"})
                   {
                       GlobalLeaf c;
                       c.kind = GlobalInit::Value;
                       c.value = v;
                       c.init = k.num(k.i32, v);
                       c.loc = k.at();
                       la.elems.push_back(c);
                   }
                   el.leaves.push_back(la);
               }
               g->elems.push_back(el);
           }
           g->rest = ArrayRest::Zero;
           Function* f {k.fn("f", k.vd)};
           auto rd = [&](const char* e, int member, const char* j)
           {
               return k.deref(k.b.elem_addr(
                   k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, e), k.at()), member, k.at()), N(k, j),
                   k.at()));
           };
           const Expr* one {N(k, "1")};
           k.req(f, k.land(k.b.global_atom(AtomKind::Reads, k.b.global_addr(g, k.at()), 0, k.at(), false),
                           k.land(k.land(k.eq(rd("0", 0, "1"), one), k.eq(rd("0", 1, "1"), one)),
                                  k.land(k.land(k.eq(rd("1", 0, "1"), one), k.eq(rd("0", 0, "0"), one)),
                                         k.eq(rd("0", 0, "1"), one)))));
           f->body = k.blk({});
           k.done(f);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   std::set<std::string> out;
                   for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
                   {
                       size_t end {at + 4U};
                       while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
                       {
                           ++end;
                       }
                       out.insert(vpr.substr(at, end - at));
                   }
                   return out.size() == 4U
                              ? ""
                              : "the ghosts of the five reads: expected 4 (one per distinct cell), found " +
                                    std::to_string(out.size());
               });
       });
    // void f(void) { static struct P sg[2] = {{7, {1, 2, 3}}, {9, {4}}}; sg[1].a[1] = 9; check(sg[1].a[1] == 9 && sg[0].a[2] == 3); }   int main() { f(); return 0; }
    const auto lstatic = [=](Kit& k, bool with_atom) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd)};
        Global* g {gs_global(k, P, false, f)};
        if (with_atom)
        {
            f->requires_.push_back(k.cl(k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), true)));
            f->ensures_.push_back(k.cl(k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), true)));
        }
        const Stmt* w {k.store(gplace(k, g, "1", "1"), N(k, "9"))};
        f->body = k.blk({w, k.chk(k.eq(k.deref(gplace(k, g, "1", "1")), N(k, "9")))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return w;
    };
    lc("e1e2e_gsarr_static_local",
       "a `static` local array of structures with an array member and its implicit owns atom: f writes sg[1].a[1] and reads it back (a `static` local's value on entry to f is not known: no other claim)",
       [=](Kit& k)
       {
           lstatic(k, true);
       });
    lc("e1e2e_gsarr_static_local_unowned_fail", "the same with no atom on sg: the write is a permission error",
       [=](Kit& k)
       {
           k.fail("permission", lstatic(k, false)->loc);
       });
    // int main(void) { int i = 0; while (i < 3) inv 0 <= i <= 3 && g[1].a[2] == 0 decreases 3 - i { i++; } check(g[1].a[2] == 0); return 0; }   (and the loop writing g[1].a[i])
    const auto lloop = [=](Kit& k, bool writes) -> const Clause
    {
        StructType* P {mk_p(k)};
        Global* g {gs_global(k, P, false)};
        Function* m {k.fn("main", k.i32)};
        Var* i {k.local(m, "i", k.i32)};
        const Clause keep {k.cl(k.eq(k.deref(gplace(k, g, "1", "2")), N(k, "0")))};
        std::vector<const Stmt*> lb;
        if (writes)
        {
            lb.push_back(k.store(
                k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "1"), k.at()), 1, k.at()),
                              k.v(i), k.at()),
                N(k, "1")));
        }
        lb.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                              {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(lb))};
        m->body = k.blkc({k.decl(i, N(k, "0")), lp, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return keep;
    };
    lc("e1e2e_gsarr_loop_reads",
       "main: a loop that only reads: the global's places (the array member over two bound variables) are carried, g[1].a[2] == 0 is kept",
       [=](Kit& k)
       {
           lloop(k, false);
       });
    lc("e1e2e_gsarr_loop_write_loses_values_fail",
       "main: the loop writes g[1].a[i]: a store reaches the member's place, so g[1].a[2] == 0 is not kept (the invariant fails at the back edge)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", lloop(k, true).loc);
       });
    // void f(void) requires(owns(g) && g[1].a[2] < 1000) ensures(owns(g) && g[1].a[2] == old(g[1].a[2]) + <d>) { g[1].a[2] = g[1].a[2] + 1; }   int main() { f(); return 0; }
    const auto old_g = [=](Kit& k, const char* d) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Global* g {gs_global(k, P, false)};
        auto atom = [&]()
        {
            return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        Function* f {k.fn("f", k.vd)};
        k.req(f, k.land(atom(), k.lt(k.deref(gplace(k, g, "1", "2")), N(k, "1000"))));
        k.ens(f, k.land(atom(),
                        k.eq(k.deref(gplace(k, g, "1", "2")), k.add(k.old(k.deref(gplace(k, g, "1", "2"))), N(k, d)))));
        const Stmt* st {k.store(gplace(k, g, "1", "2"), k.add(k.deref(gplace(k, g, "1", "2")), N(k, "1")))};
        const Stmt* bs {k.blkc({st})};
        f->body = bs;
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return f->end_loc;
    };
    lc("e1e2e_gsarr_old_read",
       "ensures g[1].a[2] == old(g[1].a[2]) + 1 after g[1].a[2] = g[1].a[2] + 1 under owns(g): the lowering takes the old read of a global array of such structures (the encoder's refusal, if any, is not the lowering's)",
       [=](Kit& k)
       {
           old_g(k, "1");
       });
    lc("e1e2e_gsarr_old_read_wrong_fail", "the same with + 2: the postcondition fails at the end of f",
       [=](Kit& k)
       {
           k.fail("postcondition", old_g(k, "2"));
       });
}

// ---- Part E, slice 3c-4 (record 0064 items 2, 3, 6, 7, 10 and 13): a MEMBER that is an array of structures whose element has an array member of scalars ----
// struct P { int n; int a[3]; } and struct T { struct P v[2]; int k; }: member 0 of T is the array of structures, member 1 the cell k; member 0 of P is the cell n, member 1 the array a.
void define_part_e_member_struct_array_leaf_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_t = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"k", k.i32}});
    };
    // `base->v[i].a[j]` as a place, and `base->v[i].n`
    auto aplace = [](Kit& k, const Expr* base, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.mem(base, 0), i, k.at()), 1, k.at()), j, k.at());
    };
    auto nplace = [](Kit& k, const Expr* base, const Expr* i)
    {
        return k.b.member_addr(k.b.elem_addr(k.mem(base, 0), i, k.at()), 0, k.at());
    };
    auto aread = [=](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.deref(aplace(k, base, N(k, i), N(k, j)));
    };
    auto awrite = [=](Kit& k, const Expr* base, const char* i, const char* j, const char* v)
    {
        return k.store(aplace(k, base, N(k, i), N(k, j)), N(k, v));
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    // every leaf of t written: v[e].n = 1 + e, v[e].a[c] = 10 * (e + 1) + c, k = kv
    auto fill = [=](Kit& k, Var* t, std::vector<const Stmt*>& body, const char* kv)
    {
        body.push_back(k.decl(t, nullptr));
        const char* ns[2] {"1", "2"};
        const char* as[2][3] {{"10", "11", "12"}, {"20", "21", "22"}};
        for (int e {0}; e < 2; ++e)
        {
            body.push_back(k.store(nplace(k, k.addr(t), N(k, e == 0 ? "0" : "1")), N(k, ns[e])));
            for (int c {0}; c < 3; ++c)
            {
                body.push_back(awrite(k, k.addr(t), e == 0 ? "0" : "1", c == 0 ? "0" : (c == 1 ? "1" : "2"), as[e][c]));
            }
        }
        body.push_back(k.lstore(k.addr(t), 1, N(k, kv)));
    };
    // void f(void) { struct T t; ...fill...; check(t.v[1].a[2] == <want> && t.v[0].a[1] == 11 && t.v[1].n == 2 && t.k == 7); t.v[1].a[2] = 9; check(t.v[1].a[2] == 9 && t.v[1].a[1] == 21); }
    const auto local = [=](Kit& k, const char* want, SrcLoc* at_check = nullptr) -> void
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        const Stmt* c {k.chk(k.land(
            k.land(k.eq(k.deref(aplace(k, k.addr(t), N(k, "1"), N(k, "2"))), N(k, want)),
                   k.eq(k.deref(aplace(k, k.addr(t), N(k, "0"), N(k, "1"))), N(k, "11"))),
            k.land(k.eq(k.deref(nplace(k, k.addr(t), N(k, "1"))), N(k, "2")), k.eq(k.leaf(k.addr(t), 1), N(k, "7")))))};
        if (at_check != nullptr)
        {
            *at_check = c->loc;
        }
        body.push_back(c);
        body.push_back(awrite(k, k.addr(t), "1", "2", "9"));
        body.push_back(k.chk(
            k.land(k.eq(aread(k, k.addr(t), "1", "2"), N(k, "9")), k.eq(aread(k, k.addr(t), "1", "1"), N(k, "21")))));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
    };
    lcv("e1e2e_tsarr_local_rw",
        "a structure local whose member is an array of structures with an array member: every leaf written and read back, the cells of the member of an element, the scope exit per leaf",
        [=](Kit& k)
        {
            local(k, "22");
        });
    lc("e1e2e_tsarr_local_wrong_fail", "the same: the check that t.v[1].a[2] is 23 fails",
       [=](Kit& k)
       {
           SrcLoc at;
           local(k, "23", &at);
           k.fail("check", at);
       });
    lc("e1e2e_tsarr_first_index_outside_fail",
       "t.v[2].a[0] of struct P v[2]: the first bound row (out-of-bounds) at the read",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           const Expr* r {aread(k, k.addr(t), "2", "0")};
           body.push_back(k.chk(k.eq(r, N(k, "0"))));
           f->body = k.blkc(body);
           k.done(f);
           k.fail_only("out-of-bounds", r->loc, 2U, 0U);
       });
    lc("e1e2e_tsarr_second_index_outside_fail",
       "t.v[1].a[3] of int a[3]: the second bound row (out-of-bounds) at the read",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           const Expr* r {aread(k, k.addr(t), "1", "3")};
           body.push_back(k.chk(k.eq(r, N(k, "0"))));
           f->body = k.blkc(body);
           k.done(f);
           k.fail_only("out-of-bounds", r->loc, 2U, 1U);
       });
    lc("e1e2e_tsarr_unwritten_fail", "t.v[1].a[1] read before any write: uninitialized at the read",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           const Expr* r {aread(k, k.addr(t), "1", "1")};
           f->body = k.blkc({k.decl(t, nullptr), awrite(k, k.addr(t), "1", "0", "3"), k.chk(k.eq(r, N(k, "0")))});
           k.done(f);
           k.fail("uninitialized", r->loc);
       });
    // the zero fill of the member (C's `struct T t = {0}`): every cell of every element zero and set, then k written
    const auto zerofill = [=](Kit& k, const char* want, SrcLoc* at_check = nullptr)
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        const Stmt* c {k.chk(k.land(k.eq(aread(k, k.addr(t), "1", "2"), N(k, want)),
                                    k.eq(k.deref(nplace(k, k.addr(t), N(k, "0"))), N(k, "0"))))};
        if (at_check != nullptr)
        {
            *at_check = c->loc;
        }
        std::vector<const Stmt*> body {k.decl(t, nullptr), k.b.array_zero(k.mem(k.addr(t), 0), k.at()),
                                       k.lstore(k.addr(t), 1, N(k, "7")), c};
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
    };
    lcv("e1e2e_tsarr_zero_fill",
        "ArrayZero over the member v: zero_arr_<P> over the member's first slot; every cell of the member of every element reads zero",
        [=](Kit& k)
        {
            zerofill(k, "0");
        });
    lc("e1e2e_tsarr_zero_fill_wrong_fail", "the zero fill: the check that t.v[1].a[2] is 1 fails",
       [=](Kit& k)
       {
           SrcLoc at;
           zerofill(k, "1", &at);
           k.fail("check", at);
       });
    // void g(struct T *p) requires(owns(*p)) ensures(owns(*p) && p->k == <post>) { p->v[1].a[2] = 7; p->k = 3; }
    auto make_g = [=](Kit& k, StructType* T, const char* post) -> Function*
    {
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(T))}})};
        k.req(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
        k.ens(g, k.land(k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()),
                        k.eq(k.leaf(k.b.text(k.F(g, 0), "p"), 1), N(k, post))));
        g->body = k.blk({awrite(k, k.pv(g, 0), "1", "2", "7"), k.chk(k.eq(aread(k, k.pv(g, 0), "1", "2"), N(k, "7"))),
                         k.lstore(k.pv(g, 0), 1, N(k, "3"))});
        k.done(g);
        return g;
    };
    // void f(void) { struct T t; ...fill...; g(&t); check(t.k == 3); }
    lcv("e1e2e_tsarr_owns_call",
        "g(&t) takes owns(*p) of a structure whose member is an array of structures with an array member: the rows per leaf over the bound local, the cells of the member of each element; the callee writes p->v[1].a[2]; the caller reads t.k back",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* g {make_g(k, T, "3")};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.es(k.call(g, {k.addr(t)})));
            body.push_back(k.chk(k.eq(k.leaf(k.addr(t), 1), N(k, "3"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2e_tsarr_owns_callee_wrong_fail",
       "g writes 3 where it promises 4: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* g {make_g(k, T, "4")};
           k.fail("postcondition", g->end_loc);
       });
    // void put(struct T *p) requires(owns(p->v)) ensures(owns(p->v)) { p->v[1].a[2] = 7; [p->k = 1;] }
    auto make_put = [=](Kit& k, StructType* T, bool writes_k) -> Function*
    {
        Function* f {k.fn("put", k.vd, {{"p", k.ptr(k.st(T))}})};
        auto atom = [&]()
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.mem(k.F(f, 0), 0), "p->v"), 0, k.at());
        };
        k.req(f, atom());
        k.ens(f, atom());
        std::vector<const Stmt*> body {awrite(k, k.pv(f, 0), "1", "2", "7")};
        const Stmt* w {nullptr};
        if (writes_k)
        {
            w = k.lstore(k.pv(f, 0), 1, N(k, "1"));
            body.push_back(w);
        }
        f->body = k.blk(body);
        k.done(f);
        if (writes_k)
        {
            k.fail("permission", w->loc);
        }
        return f;
    };
    lcv("e1e2e_tsarr_owns_member",
        "owns(p->v) alone: the whole array member and nothing else; the callee writes p->v[1].a[2]; the caller keeps its cell k",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* put {make_put(k, T, false)};
            Function* f {k.fn("use", k.vd)};
            Var* t {k.scell(f, "t", T)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.es(k.call(put, {k.addr(t)})));
            body.push_back(k.chk(k.eq(k.leaf(k.addr(t), 1), N(k, "7"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2e_tsarr_owns_member_writes_k_fail", "p->k written under owns(p->v): permission at the store",
       [=](Kit& k)
       {
           make_put(k, mk_t(k), true);
       });
    // int peek(struct T *p) requires(reads(*p)) ensures(reads(*p) && result == p->k) { [p->v[1].a[0] = 0;] return p->k; }
    auto make_peek = [=](Kit& k, StructType* T, bool writes) -> Function*
    {
        Function* f {k.fn("peek", k.i32, {{"p", k.ptr(k.st(T))}})};
        k.req(f, k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()));
        k.ens(f, k.land(k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()),
                        k.eq(k.R(f), k.leaf(k.b.text(k.F(f, 0), "p"), 1))));
        std::vector<const Stmt*> body;
        const Stmt* w {nullptr};
        if (writes)
        {
            w = awrite(k, k.pv(f, 0), "1", "0", "0");
            body.push_back(w);
        }
        body.push_back(k.ret(k.leaf(k.pv(f, 0), 1)));
        f->body = k.blk(body);
        k.done(f);
        if (writes)
        {
            k.fail("permission", w->loc);
        }
        return f;
    };
    lcv("e1e2e_tsarr_reads_call",
        "peek(&t) takes reads(*p): every cell of the member of every element at a fraction; the caller still owns t and writes t.v[1].a[1] afterwards",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* peek {make_peek(k, T, false)};
            Function* f {k.fn("use", k.vd)};
            Var* t {k.scell(f, "t", T)};
            Var* r {k.local(f, "r", k.i32)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.decl(r, k.call(peek, {k.addr(t)})));
            body.push_back(awrite(k, k.addr(t), "1", "1", "9"));
            body.push_back(k.chk(k.land(k.eq(k.v(r), N(k, "7")), k.eq(aread(k, k.addr(t), "1", "1"), N(k, "9")))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2e_tsarr_reads_write_fail", "p->v[1].a[0] written under reads(*p): permission at the store",
       [=](Kit& k)
       {
           make_peek(k, mk_t(k), true);
       });
    // a loop over the elements: struct T t; ...fill...; int i = 0; while (i < 2) inv 0 <= i <= 2 { t.v[i].a[1] = 5; i++; } check(t.v[0].a[1] == ?)
    lcv("e1e2e_tsarr_loop_fill",
        "a loop writing t.v[i].a[1]: the member's leaves are array places of the loop, each leaf its own place with its cells; the other cells stay readable after it",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            Var* i {k.local(f, "i", k.i32)};
            Var* z {k.local(f, "z", k.i32)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.decl(i, N(k, "0")));
            const Expr* place {aplace(k, k.addr(t), k.pn(i), N(k, "1"))};
            body.push_back(k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.range(k.v(i), k.i32, "0", "2"))},
                                 {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc({k.store(place, N(k, "5")), k.incS(i)})));
            body.push_back(k.decl(z, aread(k, k.addr(t), "0", "2")));
            body.push_back(k.chk(k.eq(k.v(i), N(k, "2"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // the loop through a pointer parameter: void fillv(struct T *p) requires(owns(*p)) ensures(owns(*p)) { int i = 0; while (i < 2) { p->v[i].a[1] = 5; i++; } int z = p->v[1].a[2]; }
    lcv("e1e2e_tsarr_loop_through_a_pointer",
        "owns(*p) held by a callee whose loop writes p->v[i].a[1]: the member's leaves are range places of the loop; the other cells stay readable",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* g {k.fn("fillv", k.vd, {{"p", k.ptr(k.st(T))}})};
            k.req(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
            k.ens(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
            Var* i {k.local(g, "i", k.i32)};
            Var* z {k.local(g, "z", k.i32)};
            const Expr* place {aplace(k, k.pv(g, 0), k.pn(i), N(k, "1"))};
            std::vector<const Stmt*> body {k.decl(i, N(k, "0")),
                                           k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.range(k.v(i), k.i32, "0", "2"))},
                                                 {k.cl(k.sub(N(k, "2"), k.v(i)))},
                                                 k.blkc({k.store(place, N(k, "5")), k.incS(i)})),
                                           k.decl(z, aread(k, k.pv(g, 0), "1", "2"))};
            tail(k, body);
            g->body = k.blkc(body);
            k.done(g);
        });
    // ---- the contract read p->v[e].a[e2] of an array member of such structures (question 376: NO tie, "tie STOPPED (376)": question 345's listed false alarm, pinned) ----
    auto PT = [](Kit& k, StructType* S)
    {
        return k.ptr(k.st(S));
    };
    // the atom of `shape` on the formal p: 0 owns(*p), 1 reads(*p), 2 owns(p->v), 3 reads(p->v), 4 none, 5 owns(p->k)
    auto patom = [=](Kit& k, Function* f, int shape) -> const Expr*
    {
        const Expr* p {k.b.text(k.F(f, 0), "p")};
        switch (shape)
        {
            case 0:
                return k.b.atom(AtomKind::Owns, p, 0, k.at());
            case 1:
                return k.b.atom(AtomKind::Reads, p, 0, k.at());
            case 2:
                return k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), 0), 0, k.at());
            case 3:
                return k.b.atom(AtomKind::Reads, k.mem(k.b.text(k.F(f, 0), "p"), 0), 0, k.at());
            case 5:
                return k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), 1), 0, k.at());
            default:
                return nullptr;
        }
    };
    auto with = [=](Kit& k, const Expr* atom, const Expr* c) -> const Expr*
    {
        return atom != nullptr ? k.land(atom, c) : c;
    };
    // int f(struct T *p) requires(<atom> && p->v[ri].a[rj] == 4) ensures(<atom> && r == p->v[ei].a[ej]) { return p->v[1].a[0]; }
    // main: struct T t; zero fill of v; t.k = 1; t.v[1].a[0] = 4; int r = f(&t); check(r == t.v[ei].a[ej]);
    const auto lread = [=](Kit& k, int shape, const char* ri, const char* rj, const char* ei, const char* ej,
                           SrcLoc* call_loc = nullptr, SrcLoc* req_loc = nullptr, SrcLoc* ens_loc = nullptr,
                           SrcLoc* ret_loc = nullptr, bool divide = false) -> const Stmt*
    {
        StructType* S {mk_t(k)};
        Function* f {k.fn("f", k.i32, {{"p", PT(k, S)}})};
        const Expr* rd1 {aread(k, k.F(f, 0), ri, rj)};
        const Expr* rd2 {aread(k, k.F(f, 0), ei, ej)};
        if (req_loc != nullptr)
        {
            *req_loc = rd1->loc;
        }
        const Expr* body_read {aread(k, k.pv(f, 0), "1", "0")};
        k.req(f, with(k, patom(k, f, shape), divide ? k.ne(rd1, N(k, "0")) : k.eq(rd1, N(k, "4"))));
        const Expr* ens_value {divide ? k.div(N(k, "10"), rd2) : rd2};
        if (ens_loc != nullptr)
        {
            *ens_loc = ens_value->loc;
        }
        k.ens(f, with(k, patom(k, f, shape), k.eq(k.R(f), ens_value)));
        const Stmt* ret {k.ret(divide ? k.div(N(k, "10"), body_read) : body_read)};
        if (ret_loc != nullptr)
        {
            *ret_loc = ret->loc;
        }
        f->body = k.blk({ret});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* t {k.scell(m, "t", S)};
        Var* r {k.local(m, "r", k.i32)};
        const Expr* callx {k.call(f, {k.addr(t)})};
        if (call_loc != nullptr)
        {
            *call_loc = callx->loc;
        }
        const Stmt* c {divide ? k.chk(k.yes()) : k.chk(k.eq(k.v(r), aread(k, k.addr(t), ei, ej)))};
        m->body =
            k.blkc({k.decl(t, nullptr), k.b.array_zero(k.mem(k.addr(t), 0), k.at()), k.lstore(k.addr(t), 1, N(k, "1")),
                    awrite(k, k.addr(t), "1", "0", "4"), k.decl(r, callx), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_tsarr_contract_read_owns_whole",
       "requires owns(*p) && p->v[1].a[0] == 4, ensures owns(*p) && r == p->v[1].a[0]: framed by the structure atom's array part; the caller's check of the result against t.v[1].a[0] holds",
       [=](Kit& k)
       {
           lread(k, 0, "1", "0", "1", "0");
       });
    lc("e1e2e_tsarr_contract_read_reads_whole", "the same under reads(*p)",
       [=](Kit& k)
       {
           lread(k, 1, "1", "0", "1", "0");
       });
    lc("e1e2e_tsarr_contract_read_owns_member", "the same under owns(p->v), the array member's own atom",
       [=](Kit& k)
       {
           lread(k, 2, "1", "0", "1", "0");
       });
    lc("e1e2e_tsarr_contract_read_reads_member", "the same under reads(p->v)",
       [=](Kit& k)
       {
           lread(k, 3, "1", "0", "1", "0");
       });
    lc("e1e2e_tsarr_contract_read_wrong_cell_fail",
       "requires p->v[1].a[0] == 4 but ensures r == p->v[1].a[1] under owns(*p): the body returns p->v[1].a[0], the postcondition fails",
       [=](Kit& k)
       {
           SrcLoc ret;
           lread(k, 0, "1", "0", "1", "1", nullptr, nullptr, nullptr, &ret);
           k.fail("postcondition", ret);
       });
    lc("e1e2e_tsarr_contract_read_wrong_element_fail",
       "the same with the ensures reading p->v[0].a[0]: the postcondition fails",
       [=](Kit& k)
       {
           SrcLoc ret;
           lread(k, 0, "1", "0", "0", "0", nullptr, nullptr, nullptr, &ret);
           k.fail("postcondition", ret);
       });
    lc("e1e2e_tsarr_contract_read_unframed_fail",
       "no atom on p: the read p->v[1].a[0] in the requires is held by nothing (permission at the read); main's call fails the precondition",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           SrcLoc ens;
           lread(k, 4, "1", "0", "1", "0", &call, &req, &ens);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_tsarr_contract_read_other_member_atom_fail",
       "owns(p->k) frames the member k alone: the read of the array member is held by nothing (permission)",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           SrcLoc ens;
           lread(k, 5, "1", "0", "1", "0", &call, &req, &ens);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_tsarr_contract_read_second_index_outside_fail",
       "requires owns(*p) && p->v[1].a[3] == 4: the second index is outside the member of three cells: no cell of the member is that place, permission",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           lread(k, 0, "1", "3", "1", "0", &call, &req);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_tsarr_contract_read_first_index_outside_fail",
       "requires owns(*p) && p->v[2].a[0] == 4: the element index is outside the array member of two: permission",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc req;
           lread(k, 0, "2", "0", "1", "0", &call, &req);
           k.fail("permission", req);
           k.fail("precondition", call);
       });
    lc("e1e2e_tsarr_contract_read_tie_false_alarm_fail",
       "requires reads(*p) && p->v[1].a[0] != 0, ensures r == 10 / p->v[1].a[0]: f__wf does not tie the entry and exit ghosts of a cell of the member of an element of an array member (0063 item 7's N1, question 345's listed false alarm; \"tie STOPPED (376)\" for the member form): division-by-zero at the ensures, a fail-closed false alarm, pinned",
       [=](Kit& k)
       {
           SrcLoc ens;
           lread(k, 1, "1", "0", "1", "0", nullptr, nullptr, &ens, nullptr, true);
           k.fail("division-by-zero", ens);
       });
    // struct W { struct P v[2]; struct P w[2]; }: two array members; void f(struct W *p) requires(reads(*p) && p->v[0].a[1] == 3 && p->v[1].a[0] == 4 && p->w[1].a[0] == 5 && p->v[1].a[1] == 6 && p->v[0].a[1] == 3)
    lc("e1e2e_tsarr_contract_reads_keys",
       "six reads of four distinct cells (v[0].a[1] twice) under reads(*p): the member, the element index and the cell index are each part of the key (p->v[1].a[0] and p->w[1].a[0] are two ghosts): four ghosts, none unframed",
       [=](Kit& k)
       {
           StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
           StructType* W {k.strukt("W", {{"v", k.prog.array_of(k.st(P), 2U)}, {"w", k.prog.array_of(k.st(P), 2U)}})};
           Function* f {k.fn("f", k.vd, {{"p", PT(k, W)}})};
           auto rd = [&](int member, const char* i, const char* j)
           {
               return k.deref(
                   k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.mem(k.F(f, 0), member), N(k, i), k.at()), 1, k.at()),
                                 N(k, j), k.at()));
           };
           k.req(f, k.land(k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()),
                           k.land(k.land(k.eq(rd(0, "0", "1"), N(k, "3")), k.eq(rd(0, "1", "0"), N(k, "4"))),
                                  k.land(k.land(k.eq(rd(1, "1", "0"), N(k, "5")), k.eq(rd(0, "1", "1"), N(k, "6"))),
                                         k.eq(rd(0, "0", "1"), N(k, "3"))))));
           f->body = k.blk({});
           k.done(f);
           k.text_checks.push_back(
               [=](const std::string& vpr) -> std::string
               {
                   std::set<std::string> out;
                   for (size_t at {vpr.find("hv_e")}; at != std::string::npos; at = vpr.find("hv_e", at + 1U))
                   {
                       size_t end {at + 4U};
                       while (end < vpr.size() && std::isdigit(static_cast<unsigned char>(vpr[end])) != 0)
                       {
                           ++end;
                       }
                       out.insert(vpr.substr(at, end - at));
                   }
                   return out.size() == 4U ? ""
                                           : "the ghosts of the six reads: expected 4 (one per distinct cell), found " +
                                                 std::to_string(out.size());
               });
       });
    // struct T *f(struct T *p) requires(owns(*p) && p->v[0].a[0] == 3) ensures(owns(*p) && result == p && result->v[0].a[0] == 3) { return p; }
    const auto lresult = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S {mk_t(k)};
        Function* f {k.fn("f", PT(k, S), {{"p", PT(k, S)}})};
        const Expr* pr {k.b.text(k.F(f, 0), "p")};
        k.req(f, k.land(k.b.atom(AtomKind::Owns, pr, 0, k.at()), k.eq(aread(k, k.F(f, 0), "0", "0"), N(k, "3"))));
        k.ens(f, k.land(k.land(k.b.atom(AtomKind::Owns, pr, 0, k.at()), k.eq(k.R(f), k.F(f, 0))),
                        k.eq(aread(k, k.R(f), "0", "0"), N(k, wrong ? "4" : "3"))));
        const Stmt* ret {k.ret(k.pv(f, 0))};
        f->body = k.blk({ret});
        k.done(f);
        return ret;
    };
    lc("e1e2e_tsarr_contract_read_result",
       "an ensures that reads result->v[0].a[0] under owns(*p) && result == p: the result is the parameter, whose atom frames the read (the structure's array part); the body returns p, which has not been written",
       [=](Kit& k)
       {
           lresult(k, false);
       });
    lc("e1e2e_tsarr_contract_read_result_wrong_fail",
       "the same ensuring result->v[0].a[0] == 4 where the requires said 3: the postcondition fails at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", lresult(k, true)->loc);
       });
    // void keep(struct T *p) requires(owns(*p)) ensures(owns(*p) && p->v[1].a[2] == old(p->v[1].a[2])) { [p->v[1].a[2] = 5;] }
    auto keep_old = [=](Kit& k, bool writes)
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("keep", k.vd, {{"p", PT(k, T)}})};
        auto own = [&]()
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
        };
        k.req(f, own());
        k.ens(f, k.land(own(), k.eq(aread(k, k.F(f, 0), "1", "2"), k.old(aread(k, k.F(f, 0), "1", "2")))));
        f->body = writes ? k.blk({awrite(k, k.pv(f, 0), "1", "2", "5")}) : k.blk({});
        k.done(f);
        if (writes)
        {
            k.fail("postcondition", f->end_loc);
        }
    };
    lc("e1e2e_tsarr_contract_old", "ensures p->v[1].a[2] == old(p->v[1].a[2]) of a function that writes nothing: holds",
       [=](Kit& k)
       {
           keep_old(k, false);
       });
    lc("e1e2e_tsarr_contract_old_changed_fail",
       "the same function writing p->v[1].a[2] = 5: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           keep_old(k, true);
       });
    // void keep(int *p, int n) requires(owns_range(p, n)); void f(void) { struct T t; ...fill...; keep(&t.v[1].a[0], 3); }: the callee keeps the cells of the member of an element
    lc("e1e2e_tsarr_cells_kept_fail",
       "the cells t.v[1].a[0..3) given to a callee that keeps them: lifetime at the scope exit (the array leaf's value permission)",
       [=](Kit& k)
       {
           Function* g {k.fn("keep", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.trusted(g);
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           body.push_back(k.es(k.call(g, {aplace(k, k.addr(t), N(k, "1"), N(k, "0")), N(k, "3")})));
           const Stmt* bs {k.blkc(body)};
           f->body = bs;
           k.done(f);
           // rows: t.k, then n's value and flag, the array leaf's value permission (taken by the callee: it ends the path) and flag, the liveness
           k.fail_only("lifetime", bs->end_loc, 6U, 3U);
       });
    // void keep(int *p, int n) requires(owns_range(p, n)); void f(void) { struct T t; ...fill...; keep(&t.v[1].a[2], 1); }: the callee keeps only the LAST cell of the member of an element
    lc("e1e2e_tsarr_last_cell_kept_fail",
       "the one cell t.v[1].a[2] given to a callee that keeps it: lifetime at the scope exit (the marked row of the array leaf covers every cell of the member, not the first alone)",
       [=](Kit& k)
       {
           Function* g {k.fn("keep", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.trusted(g);
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           body.push_back(k.es(k.call(g, {aplace(k, k.addr(t), N(k, "1"), N(k, "2")), N(k, "1")})));
           const Stmt* bs {k.blkc(body)};
           f->body = bs;
           k.done(f);
           k.fail_only("lifetime", bs->end_loc, 6U, 3U);
       });
    // void hand(int *p, int n) requires(owns_range(p, n)) ensures(owns_range(p, n)) { }; main: fill; hand(&t.v[1].a[0], 3); t.v[1].a[2] = 4;
    lc("e1e2e_tsarr_cells_given_and_returned",
       "the cells t.v[1].a[0..3) given to a callee that hands them back: the caller writes t.v[1].a[2] after the call",
       [=](Kit& k)
       {
           Function* g {k.fn("hand", k.vd, {{"p", k.ptr(k.i32)}, {"n", k.i32}})};
           k.req(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           k.ens(g, k.b.range_atom(AtomKind::OwnsRange, k.F(g, 0), k.F(g, 1), 0, k.at()));
           g->body = k.blk({});
           k.done(g);
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           body.push_back(k.es(k.call(g, {aplace(k, k.addr(t), N(k, "1"), N(k, "0")), N(k, "3")})));
           body.push_back(awrite(k, k.addr(t), "1", "2", "4"));
           f->body = k.blkc(body);
           k.done(f);
       });
    // ---- pairing (item 7) and sequencing (item 11): a call that holds the array member beside a cell or another call ----
    // void mix(struct T *p, int *q) requires(owns(p->v) && owns(*q)) ensures(owns(p->v) && owns(*q)) {}
    const auto mix = [=](Kit& k, StructType* S) -> Function*
    {
        Function* g {k.fn("mix", k.vd, {{"p", PT(k, S)}, {"q", k.ptr(k.i32)}})};
        auto both = [&]()
        {
            return k.land(k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(g, 0), "p"), 0), 0, k.at()),
                          k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 1), "q"), 1, k.at()));
        };
        k.req(g, both());
        k.ens(g, both());
        k.trusted(g);
        return g;
    };
    // void f(void) { struct T t; ...fill...; mix(&t, <&t.k | &t.v[1].a[2]>); }
    const auto call_mix = [=](Kit& k, bool inside) -> const Expr*
    {
        StructType* S {mk_t(k)};
        Function* g {mix(k, S)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", S)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        const Expr* callx {
            k.call(g, {k.addr(t), inside ? aplace(k, k.addr(t), N(k, "1"), N(k, "2")) : k.mem(k.addr(t), 1)})};
        body.push_back(k.es(callx));
        f->body = k.blkc(body);
        k.done(f);
        return callx;
    };
    lc("e1e2e_tsarr_pair_member_and_cell_disjoint",
       "mix(&t, &t.k) with owns(p->v) && owns(*q): the cell t.k is outside the array member",
       [=](Kit& k)
       {
           call_mix(k, false);
       });
    lc("e1e2e_tsarr_pair_member_and_cell_alias_fail",
       "mix(&t, &t.v[1].a[2]) with owns(p->v) && owns(*q): the cell is a cell of the member of an element (permission, at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", call_mix(k, true)->loc, 26U, 12U);
       });
    // int put(struct T *p) requires(owns(p->v)) ensures(owns(p->v) && r == 1) { return 1; }   void f(void) { struct T t; ...fill...; int z = put(&t) + <rhs>; }
    const auto tseq = [=](Kit& k, int mode) -> const Expr*
    {
        StructType* S {mk_t(k)};
        Function* put {k.fn("put", k.i32, {{"p", PT(k, S)}})};
        k.req(put, k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(put, 0), "p"), 0), 0, k.at()));
        k.ens(put, k.land(k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(put, 0), "p"), 0), 0, k.at()),
                          k.eq(k.R(put), N(k, "1"))));
        put->body = k.blk({k.ret(N(k, "1"))});
        k.done(put);
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", S)};
        Var* z {k.local(f, "z", k.i32)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        const Expr* callx {k.call(put, {k.addr(t)})};
        // mode 0: beside a cell of t.v[1].a (overlap), 1: beside t.k (disjoint), 2: beside a second call (overlap), 3: beside t.v[1].n (overlap: the member holds every leaf)
        const Expr* rhs {mode == 0   ? aread(k, k.addr(t), "1", "2")
                         : mode == 1 ? k.leaf(k.addr(t), 1)
                         : mode == 3 ? k.deref(nplace(k, k.addr(t), N(k, "1")))
                                     : k.call(put, {k.addr(t)})};
        body.push_back(k.decl(z, k.add(callx, rhs)));
        f->body = k.blkc(body);
        k.done(f);
        return rhs;
    };
    lc("e1e2e_tsarr_seq_disjoint", "put(&t) + t.k: the call holds t.v, t.k is another object: no row fails",
       [=](Kit& k)
       {
           tseq(k, 1);
       });
    lc("e1e2e_tsarr_seq_call_and_cell_overlap_fail",
       "put(&t) + t.v[1].a[2]: the call holds every cell of the member of every element and the read names one of them (unsequenced)",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", tseq(k, 0)->loc, 2U, 1U);
       });
    lc("e1e2e_tsarr_seq_call_and_leaf_overlap_fail",
       "put(&t) + t.v[1].n: the call holds every leaf of the array member, the cell n of an element is one of them (unsequenced)",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", tseq(k, 3)->loc, 2U, 0U);
       });
    lc("e1e2e_tsarr_seq_two_calls_overlap_fail", "put(&t) + put(&t): two calls that both hold t.v (unsequenced)",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", tseq(k, 2)->loc, 4U, 0U);
       });
}

void define_part_e_member_struct_array_leaf_refusals()
{
    // struct P { int n; int a[3]; } and struct T { struct P v[2]; int k; }: an array member of structures whose element has an array member of scalars
    auto TM = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"k", k.i32}});
    };
    auto fp = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // ---- question 348: a structure with an array member of structures with an array member stays rejected at each of item 15's six positions (the array member's element is a structure with an array member); each is the copy of Part D's case ----
    const char* const kAgg {"decision 0064 item 15, question 348"};
    rc("e1e2e_rej_tsarr_owns_block", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p) requires(owns_block(p)); struct T { struct P v[2]; int k; }
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_marker_on_a_pointer", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::must_init]]) requires(owns(*p)) ensures(owns(*p))
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::MustInit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_array_beside_an_owning_member", kAgg,
       [=](Kit& k)
       {
           // struct R { struct P v[2]; int *q [[cerastes::owning]]; }
           StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
           StructType* R {k.strukt("R", {{"v", k.prog.array_of(k.st(P), 2U)}, {"q", k.ptr(k.i32)}})};
           R->members[1].owning = true;
       });
    rc("e1e2e_rej_tsarr_owning_member_to_such_a_structure", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q [[cerastes::owning]]; int z; }
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}, {"z", k.i32}})};
           R->members[0].owning = true;
       });
    rc("e1e2e_rej_tsarr_owning_global", kAgg,
       [=](Kit& k)
       {
           // struct T *g [[cerastes::owning]]; struct T { struct P v[2]; int k; }
           StructType* T {TM(k)};
           Global* g {k.prog.add_global("g", k.ptr(k.st(T)), k.at())};
           g->init_kind = Global::Init::Null;
           g->owning = true;
           Function* f {k.fn("f", k.vd)};
           f->body = k.blkc({});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_owning_local", kAgg,
       [=](Kit& k)
       {
           // void f(void) { struct T *q [[cerastes::owning]] = NULL; }
           StructType* T {TM(k)};
           const Type TP {k.ptr(k.st(T))};
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", TP)};
           q->owning = true;
           f->body = k.blkc({k.decl(q, k.nul(TP))});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_owning_formal_implicit_atom", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::owning]]): the implicit atom owns(*p) of an owning formal
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           const Expr* a {k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at())};
           const_cast<Expr*>(a)->implicit_atom = true;
           k.req(f, a);
           f->body = k.blkc({});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_atom_through_a_heap_read", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(owns(*r->q))
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.deref(k.mem(k.F(f, 0), 0)), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_element_read_through_a_heap_read",
       "an element read in a contract of an array member whose structure is not a pointer parameter",
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(r->q->v[1].a[1] == 0)
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           const Expr* loaded {k.deref(k.mem(k.F(f, 0), 0))};
           const Expr* el {k.b.elem_addr(k.mem(loaded, 0), k.num(k.i32, "1"), k.at())};
           const Expr* row {k.b.member_addr(el, 1, k.at())};
           k.req(f, k.eq(k.deref(k.b.elem_addr(row, k.num(k.i32, "1"), k.at())), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_partnerless_result", kAgg,
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) (no body)
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    rc("e1e2e_rej_tsarr_ref_to_uninit_local", kAgg,
       [=](Kit& k)
       {
           // struct T *p [[cerastes::ref_to_uninit]] = null;
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd)};
           Var* p {k.local(f, "p", k.ptr(k.st(T)))};
           p->ref_to_uninit = true;
           f->body = k.blk({k.decl(p, k.nul(k.ptr(k.st(T))))});
           k.done(f);
       });
    // ---- the lowering's own stop at each Part D site that takes an aggregate element (the harness cannot pin an exit 3, so each is a refused program lowered anyway) ----
    ru("e1e2e_ru_tsarr_block_lines_of_an_aggregate_array_structure", "the lines of an owns_block",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_block_rows_of_an_aggregate_array_structure", "the rows of a block",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           k.ens(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_block_framing_of_an_aggregate_array_structure", "the framing of a contract read",
       [=](Kit& k)
       {
           // void f(struct T *p) requires(owns_block(p) && p->v[0].a[0] == 0)
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           const Expr* el {k.b.elem_addr(k.mem(fp(k, f, 0), 0), k.num(k.i32, "0"), k.at())};
           const Expr* mm {k.b.member_addr(el, 1, k.at())};
           k.req(f, k.land(k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()),
                           k.eq(k.deref(k.b.elem_addr(mm, k.num(k.i32, "0"), k.at())), k.num(k.i32, "0"))));
           f->body = k.blk({});
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_block_call_record_of_an_aggregate_array_structure",
       "the sequencing record of a call's block site",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
           Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(T))}})};
           k.req(g, k.b.atom(AtomKind::OwnsBlock, fp(k, g, 0), -1, k.at()));
           g->body = k.blk({k.es(k.call(f, {k.pv(g, 0)}))});
           k.done(g);
       });
    ru("e1e2e_ru_tsarr_marked_atom_array_part_of_an_aggregate", "the array part of a marked or partnerless atom",
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::ref_to_uninit]]) requires(owns(*p)) ensures(owns(*p)) {}
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_marked_return_rows_of_an_aggregate", "the return rows of a marked or partnerless atom",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_marked_call_rows_of_an_aggregate", "the call rows of an atom under an initialization marker",
       [=](Kit& k)
       {
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
           Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(T))}})};
           k.req(g, k.b.atom(AtomKind::Owns, fp(k, g, 0), 0, k.at()));
           k.ens(g, k.b.atom(AtomKind::Owns, fp(k, g, 0), 0, k.at()));
           g->body = k.blkc({k.es(k.call(f, {k.pv(g, 0)}))});
           k.done(g);
       });
    ru("e1e2e_ru_tsarr_partnerless_array_part_of_an_aggregate", "the array part of a marked or partnerless atom",
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) (no body)
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_partnerless_return_rows_of_an_aggregate", "the return rows of a marked or partnerless atom",
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) { return null; }
           StructType* T {TM(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           f->body = k.blk({k.ret(k.nul(k.ptr(k.st(T))))});
           k.done(f);
       });
    ru("e1e2e_ru_tsarr_contract_read_through_a_heap_read_of_an_aggregate",
       "the address of an array element in a contract reached lowering",
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(r->q->v[1].a[1] == 0)
           StructType* T {TM(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           const Expr* loaded {k.deref(k.mem(k.F(f, 0), 0))};
           const Expr* el {k.b.elem_addr(k.mem(loaded, 0), k.num(k.i32, "1"), k.at())};
           const Expr* row {k.b.member_addr(el, 1, k.at())};
           k.req(f, k.eq(k.deref(k.b.elem_addr(row, k.num(k.i32, "1"), k.at())), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    // a written atom in a loop invariant on a structure with such an array member, or on the array member (the sixth position)
    const auto written_m = [=](Kit& k, bool member_array)
    {
        StructType* S {TM(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
        k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* atom {member_array ? k.b.atom(AtomKind::Owns, k.mem(k.pv(f, 0), 0), 1, k.at())
                                       : k.b.atom(AtomKind::Owns, k.pv(f, 0), 0, k.at())};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.num(k.i32, "4")), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "4"), atom))},
                              {k.cl(k.sub(k.num(k.i32, "4"), k.v(i)))},
                              k.blkc({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))};
        f->body = k.blkc({k.decl(i, k.num(k.i32, "0")), lp});
        k.done(f);
    };
    rc("e1e2e_rej_tsarr_written_owns_structure",
       "a permission atom on a structure with an array member of structures or arrays in a loop invariant",
       [=](Kit& k)
       {
           written_m(k, false);
       });
    rc("e1e2e_rej_tsarr_written_owns_member", "in a loop invariant",
       [=](Kit& k)
       {
           written_m(k, true);
       });
    ru("e1e2e_ru_tsarr_written_owns_structure",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written_m(k, false);
       });
    ru("e1e2e_ru_tsarr_written_owns_member",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written_m(k, true);
       });
}

// ---- Part E, slice 3c-4: a GLOBAL structure whose member is an array of structures with an array member of scalars: struct T gt = {{{7, {1, 2, 3}}, {9, {4}}}, 5} ----
void define_part_e_global_member_struct_array_leaf_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_t = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"k", k.i32}});
    };
    // struct T gt = {{{7, {1, 2, 3}}, {9, {4}}}, 5}  (`unknown`: the linkage and escape case, every value unknown)
    const auto gt_global = [=](Kit& k, bool unknown, const Function* owner = nullptr) -> Global*
    {
        StructType* T {mk_t(k)};
        Global* g {k.prog.add_global("gt", k.st(T), k.at(), owner)};
        const GlobalInit kind {unknown ? GlobalInit::Unknown : GlobalInit::Value};
        const ArrayRest rest {unknown ? ArrayRest::Unknown : ArrayRest::Zero};
        GlobalLeaf lv;
        lv.kind = kind;
        lv.rest = rest;
        lv.loc = k.at();
        const std::vector<std::pair<const char*, std::vector<const char*>>> els {{"7", {"1", "2", "3"}}, {"9", {"4"}}};
        for (const auto& [nv, cells] : els)
        {
            GlobalLeaf el;
            el.kind = kind;
            el.loc = k.at();
            GlobalLeaf ln;
            ln.kind = kind;
            ln.init = k.num(k.i32, nv);
            ln.value = nv;
            ln.loc = k.at();
            el.leaves.push_back(ln);
            GlobalLeaf la;
            la.kind = kind;
            la.rest = rest;
            la.loc = k.at();
            for (const char* v : cells)
            {
                GlobalLeaf c;
                c.kind = kind;
                c.value = v;
                c.init = k.num(k.i32, v);
                c.loc = k.at();
                la.elems.push_back(c);
            }
            el.leaves.push_back(la);
            lv.elems.push_back(el);
        }
        g->leaves.push_back(lv);
        GlobalLeaf lk;
        lk.kind = kind;
        lk.init = k.num(k.i32, "5");
        lk.value = "5";
        lk.loc = k.at();
        g->leaves.push_back(lk);
        return g;
    };
    auto gaplace = [](Kit& k, const Global* g, const char* i, const char* j)
    {
        return k.b.elem_addr(
            k.b.member_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 0), k.num(k.i32, i), k.at()), 1, k.at()),
            k.num(k.i32, j), k.at());
    };
    auto gnplace = [](Kit& k, const Global* g, const char* i)
    {
        return k.b.member_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 0), k.num(k.i32, i), k.at()), 0, k.at());
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the global's rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto main_end = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
        body.push_back(k.ret(k.num(k.i32, "0")));
    };
    // int main(void) { check(gt.v[0].n == 7 && gt.v[0].a[1] == 2 && gt.v[1].n == 9 && gt.v[1].a[0] == 4 && gt.v[1].a[2] == <want> && gt.k == 5); [gt.v[1].a[2] = 5; check(gt.v[1].a[2] == 5 && gt.v[0].a[0] == 1);] return 0; }
    const auto main_rw = [=](Kit& k, const char* want, bool write, bool unknown) -> const Stmt*
    {
        Global* g {gt_global(k, unknown)};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c1 {k.chk(k.land(
            k.land(k.eq(k.deref(gnplace(k, g, "0")), N(k, "7")), k.eq(k.deref(gaplace(k, g, "0", "1")), N(k, "2"))),
            k.land(
                k.land(k.eq(k.deref(gnplace(k, g, "1")), N(k, "9")), k.eq(k.deref(gaplace(k, g, "1", "0")), N(k, "4"))),
                k.land(k.eq(k.deref(gaplace(k, g, "1", "2")), N(k, want)),
                       k.eq(k.leaf(k.b.global_addr(g, k.at()), 1), N(k, "5"))))))};
        std::vector<const Stmt*> body {c1};
        if (write)
        {
            body.push_back(k.store(gaplace(k, g, "1", "2"), N(k, "5")));
            body.push_back(k.chk(k.land(k.eq(k.deref(gaplace(k, g, "1", "2")), N(k, "5")),
                                        k.eq(k.deref(gaplace(k, g, "0", "0")), N(k, "1")))));
        }
        main_end(k, body);
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return c1;
    };
    lcv("e1e2e_gtsarr_main_read",
        "struct T gt = {{{7, {1, 2, 3}}, {9, {4}}}, 5}: main reads the listed cells of the member of each element of the array member and the zero rest",
        [=](Kit& k)
        {
            main_rw(k, "0", false, false);
        });
    lc("e1e2e_gtsarr_main_read_wrong_fail", "the same: the check that gt.v[1].a[2] is 1 fails (the zero rest)",
       [=](Kit& k)
       {
           k.fail("check", main_rw(k, "1", false, false)->loc);
       });
    lcv("e1e2e_gtsarr_main_write",
        "gt.v[1].a[2] = 5 in main: the member's cells are held with write permission from program_start, the other cells keep their values",
        [=](Kit& k)
        {
            main_rw(k, "0", true, false);
        });
    lc("e1e2e_gtsarr_unknown_fail",
       "an unknown initial value (linkage): the check that gt.v[0].a[0] is 1 is not provable",
       [=](Kit& k)
       {
           Global* g {gt_global(k, true)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq(k.deref(gaplace(k, g, "0", "0")), N(k, "1")))};
           m->body = k.blkc({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    // void put(void) requires(owns(gt)) ensures(owns(gt)) { gt.v[1].a[2] = 7; }   int main() { put(); gt.v[0].a[0] = 9; check(gt.k == 5); }
    const auto call_put = [=](Kit& k, bool with_atom) -> const Stmt*
    {
        Global* g {gt_global(k, false)};
        auto atom = [&]()
        {
            return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        Function* put {k.fn("put", k.vd)};
        if (with_atom)
        {
            k.req(put, atom());
            k.ens(put, atom());
        }
        const Stmt* w {k.store(gaplace(k, g, "1", "2"), N(k, "7"))};
        put->body = k.blk({w});
        k.done(put);
        Function* m {k.fn("main", k.i32)};
        std::vector<const Stmt*> body {k.es(k.call(put, {})), k.store(gaplace(k, g, "0", "0"), N(k, "9")),
                                       k.lstore(k.b.global_addr(g, k.at()), 1, N(k, "6")),
                                       k.chk(k.land(k.eq(k.leaf(k.b.global_addr(g, k.at()), 1), N(k, "6")),
                                                    k.eq(k.deref(gaplace(k, g, "0", "0")), N(k, "9"))))};
        main_end(k, body);
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return w;
    };
    lcv("e1e2e_gtsarr_call_owns",
        "main calls put() with owns(gt): the callee writes gt.v[1].a[2]; main writes gt.v[0].a[0] and gt.k after the call (the cells come back) and reads them",
        [=](Kit& k)
        {
            call_put(k, true);
        });
    lc("e1e2e_gtsarr_call_owns_value_unknown_fail",
       "owns(gt) promises no value: check(gt.v[1].a[2] == 7) after the call fails (the callee's write is not in its contract)",
       [=](Kit& k)
       {
           Global* g {gt_global(k, false)};
           Function* put {k.fn("put", k.vd)};
           auto atom = [&]()
           {
               return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
           };
           k.req(put, atom());
           k.ens(put, atom());
           put->body = k.blk({k.store(gaplace(k, g, "1", "2"), N(k, "7"))});
           k.done(put);
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq(k.deref(gaplace(k, g, "1", "2")), N(k, "7")))};
           m->body = k.blkc({k.es(k.call(put, {})), c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2e_gtsarr_unowned_write_fail",
       "the same callee with no atom on gt: the write is a permission error at the store",
       [=](Kit& k)
       {
           k.fail("permission", call_put(k, false)->loc);
       });
    // int f(void) requires(<atom>(gt) && gt.v[ri].a[rj] != 0) ensures(<atom>(gt) && r == 10 / gt.v[ei].a[ej]) { return 10 / gt.v[ri].a[rj]; }   int main() { return f(); }
    const auto lgread = [=](Kit& k, bool with_atom, bool reads, const char* ri, const char* rj, const char* ei,
                            const char* ej, SrcLoc* call_loc = nullptr, SrcLoc* fn_loc = nullptr) -> const Expr*
    {
        Global* g {gt_global(k, false)};
        Function* f {k.fn("f", k.i32)};
        const AtomKind kind {reads ? AtomKind::Reads : AtomKind::Owns};
        auto atom = [&]()
        {
            return k.b.global_atom(kind, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        const Expr* rd1 {k.deref(gaplace(k, g, ri, rj))};
        if (fn_loc != nullptr)
        {
            *fn_loc = rd1->loc;
        }
        const Expr* pre {k.ne(rd1, N(k, "0"))};
        const Expr* d {k.div(N(k, "10"), k.deref(gaplace(k, g, ei, ej)))};
        k.req(f, with_atom ? k.land(atom(), pre) : pre);
        k.ens(f, with_atom ? k.land(atom(), k.eq(k.R(f), d)) : k.eq(k.R(f), d));
        f->body = k.blk({k.ret(k.div(N(k, "10"), k.deref(gaplace(k, g, ri, rj))))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(f, {})};
        if (call_loc != nullptr)
        {
            *call_loc = callx->loc;
        }
        m->body = k.blk({k.ret(callx)});
        k.done(m);
        k.prog.set_entry(m);
        return d;
    };
    lc("e1e2e_gtsarr_contract_read_tie_false_alarm_fail",
       "requires reads(gt) && gt.v[1].a[0] != 0, ensures r == 10 / gt.v[1].a[0]: no tie of the two states of a cell of the member of an element of an array member of a global structure (question 345's listed false alarm; \"tie STOPPED (376)\" for the member form): division-by-zero at the ensures, pinned",
       [=](Kit& k)
       {
           k.fail("division-by-zero", lgread(k, true, true, "1", "0", "1", "0")->loc);
       });
    lc("e1e2e_gtsarr_contract_read_unframed_fail",
       "the same with no atom on gt: the read gt.v[1].a[0] in the requires is held by nothing (permission at the read) and main's precondition",
       [=](Kit& k)
       {
           SrcLoc call;
           SrcLoc fn;
           lgread(k, false, false, "1", "0", "1", "0", &call, &fn);
           k.fail("precondition", call);
           k.fail("permission", fn);
       });
    // int f(void) requires(owns(gt) && gt.v[1].a[0] == <want>) ensures(owns(gt) && r == gt.v[1].a[0]) { return gt.v[1].a[0]; }   int main() { return f(); }
    const auto lgeq = [=](Kit& k, const char* want, const char* post_cell, SrcLoc* call_loc, SrcLoc* ret_loc) -> void
    {
        Global* g {gt_global(k, false)};
        Function* f {k.fn("f", k.i32)};
        auto atom = [&]()
        {
            return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        k.req(f, k.land(atom(), k.eq(k.deref(gaplace(k, g, "1", "0")), N(k, want))));
        k.ens(f, k.land(atom(), k.eq(k.R(f), k.deref(gaplace(k, g, "1", post_cell)))));
        const Stmt* ret {k.ret(k.deref(gaplace(k, g, "1", "0")))};
        *ret_loc = ret->loc;
        f->body = k.blk({ret});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        const Expr* callx {k.call(f, {})};
        *call_loc = callx->loc;
        m->body = k.blk({k.ret(callx)});
        k.done(m);
        k.prog.set_entry(m);
    };
    lc("e1e2e_gtsarr_contract_read_value",
       "requires owns(gt) && gt.v[1].a[0] == 4 (the initial value), ensures r == gt.v[1].a[0]: main holds 4 from program_start; the read is framed by the structure atom's array part",
       [=](Kit& k)
       {
           SrcLoc c;
           SrcLoc r;
           lgeq(k, "4", "0", &c, &r);
       });
    lc("e1e2e_gtsarr_contract_read_value_wrong_fail",
       "the same requiring gt.v[1].a[0] == 5: main's call fails the precondition (the initial value is 4)",
       [=](Kit& k)
       {
           SrcLoc c;
           SrcLoc r;
           lgeq(k, "5", "0", &c, &r);
           k.fail("precondition", c);
       });
    lc("e1e2e_gtsarr_contract_read_wrong_cell_fail",
       "ensures r == gt.v[1].a[1] where the body returns gt.v[1].a[0]: the postcondition fails at the return",
       [=](Kit& k)
       {
           SrcLoc c;
           SrcLoc r;
           lgeq(k, "4", "1", &c, &r);
           k.fail("postcondition", r);
       });
    // void f(void) requires(owns(gt) && gt.v[1].a[2] < 1000) ensures(owns(gt) && gt.v[1].a[2] == old(gt.v[1].a[2]) + <d>) { gt.v[1].a[2] = gt.v[1].a[2] + 1; }   int main() { f(); return 0; }
    const auto old_g = [=](Kit& k, const char* d) -> SrcLoc
    {
        Global* g {gt_global(k, false)};
        auto atom = [&]()
        {
            return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        Function* f {k.fn("f", k.vd)};
        k.req(f, k.land(atom(), k.lt(k.deref(gaplace(k, g, "1", "2")), N(k, "1000"))));
        k.ens(f, k.land(atom(), k.eq(k.deref(gaplace(k, g, "1", "2")),
                                     k.add(k.old(k.deref(gaplace(k, g, "1", "2"))), N(k, d)))));
        f->body = k.blkc({k.store(gaplace(k, g, "1", "2"), k.add(k.deref(gaplace(k, g, "1", "2")), N(k, "1")))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return f->end_loc;
    };
    lc("e1e2e_gtsarr_old_read",
       "ensures gt.v[1].a[2] == old(gt.v[1].a[2]) + 1 after gt.v[1].a[2] = gt.v[1].a[2] + 1 under owns(gt): the lowering takes the old read of a global structure's member array of such structures (the encoder's refusal, if any, is not the lowering's)",
       [=](Kit& k)
       {
           old_g(k, "1");
       });
    lc("e1e2e_gtsarr_old_read_wrong_fail", "the same with + 2: the postcondition fails at the end of f",
       [=](Kit& k)
       {
           k.fail("postcondition", old_g(k, "2"));
       });
    // int main(void) { int i = 0; while (i < 3) inv 0 <= i <= 3 && gt.v[1].a[2] == 0 decreases 3 - i { [gt.v[1].a[i] = 1;] i++; } check(gt.v[1].a[2] == 0); return 0; }
    const auto lloop = [=](Kit& k, bool writes) -> const Clause
    {
        Global* g {gt_global(k, false)};
        Function* m {k.fn("main", k.i32)};
        Var* i {k.local(m, "i", k.i32)};
        const Clause keep {k.cl(k.eq(k.deref(gaplace(k, g, "1", "2")), N(k, "0")))};
        std::vector<const Stmt*> lb;
        if (writes)
        {
            lb.push_back(k.store(
                k.b.elem_addr(
                    k.b.member_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 0), N(k, "1"), k.at()), 1, k.at()),
                    k.v(i), k.at()),
                N(k, "1")));
        }
        lb.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                              {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(lb))};
        m->body = k.blkc({k.decl(i, N(k, "0")), lp, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return keep;
    };
    lc("e1e2e_gtsarr_loop_reads",
       "main: a loop that only reads: the global structure's places (the array member's leaves over two bound variables) are carried, gt.v[1].a[2] == 0 is kept",
       [=](Kit& k)
       {
           lloop(k, false);
       });
    lc("e1e2e_gtsarr_loop_write_loses_values_fail",
       "main: the loop writes gt.v[1].a[i]: a store reaches the member's place, so gt.v[1].a[2] == 0 is not kept (the invariant fails at the back edge)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", lloop(k, true).loc);
       });
}

// ---- Part E, slice 3c-3u (the coordinator's addition; cases only): program_start's zero quantifier for the ELEMENTS an array global's initializer does not reach, and a bool array leaf with unknown values ----
void define_part_e_struct_array_leaf_unreached_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    // struct P g[3] = {{7, {1, 2, 3}}};   (element 0 only: elements 1 and 2 are the zero rest; `owner`: the `static` local form of `main`)
    const auto g3 = [=](Kit& k, StructType* P, const Function* owner) -> Global*
    {
        Global* g {k.prog.add_global("g", k.prog.array_of(k.st(P), 3U), k.at(), owner)};
        GlobalLeaf el;
        el.kind = GlobalInit::Value;
        el.loc = k.at();
        GlobalLeaf ln;
        ln.kind = GlobalInit::Value;
        ln.init = k.num(k.i32, "7");
        ln.value = "7";
        ln.loc = k.at();
        el.leaves.push_back(ln);
        GlobalLeaf la;
        la.kind = GlobalInit::Value;
        la.rest = ArrayRest::Zero;
        la.loc = k.at();
        for (const char* v : {"1", "2", "3"})
        {
            GlobalLeaf c;
            c.kind = GlobalInit::Value;
            c.value = v;
            c.init = k.num(k.i32, v);
            c.loc = k.at();
            la.elems.push_back(c);
        }
        el.leaves.push_back(la);
        g->elems.push_back(el);
        g->rest = ArrayRest::Zero;
        return g;
    };
    auto gplace = [](Kit& k, const Global* g, const char* i, const char* j)
    {
        return k.b.elem_addr(
            k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), k.num(k.i32, i), k.at()), 1, k.at()),
            k.num(k.i32, j), k.at());
    };
    auto gn = [](Kit& k, const Global* g, const char* i)
    {
        return k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), k.num(k.i32, i), k.at()), 0, k.at());
    };
    // int main(void) { [static] struct P g[3] = {{7, {1, 2, 3}}}; check(g[2].a[1] == <want> && g[0].a[2] == 3 && g[1].n == 0 && g[2].a[2] == 0); [check(false);] return 0; }
    const auto main_g3 = [=](Kit& k, const char* want, bool vacuity, bool as_static) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* m {k.fn("main", k.i32)};
        Global* g {g3(k, P, as_static ? m : nullptr)};
        const Stmt* c {k.chk(k.land(
            k.land(k.eq(k.deref(gplace(k, g, "2", "1")), N(k, want)), k.eq(k.deref(gplace(k, g, "0", "2")), N(k, "3"))),
            k.land(k.eq(k.deref(gn(k, g, "1")), N(k, "0")), k.eq(k.deref(gplace(k, g, "2", "2")), N(k, "0")))))};
        const Stmt* v {k.chk(k.no())};
        std::vector<const Stmt*> body {c};
        if (vacuity)
        {
            body.push_back(v);
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return vacuity ? v : c;
    };
    lc("e1e2e_gsarr3_unreached_read",
       "struct P g[3] = {{7, {1, 2, 3}}}: main reads g[2].a[1] == 0, g[2].a[2] == 0 and g[1].n == 0 (the elements the initializer does not reach are the zero rest) and g[0].a[2] == 3",
       [=](Kit& k)
       {
           main_g3(k, "0", false, false);
       });
    lc("e1e2e_gsarr3_unreached_wrong_fail",
       "the same: the check that g[2].a[1] is 1 fails (the unreached elements are zero)",
       [=](Kit& k)
       {
           k.fail("check", main_g3(k, "1", false, false)->loc);
       });
    lc("e1e2e_gsarr3_unreached_vacuity_fail",
       "program_start's facts for the unreached elements are satisfiable: a check(false) after the reads is reached and fails",
       [=](Kit& k)
       {
           k.fail("check", main_g3(k, "0", true, false)->loc);
       });
    lc("e1e2e_gsarr3_static_unreached_read",
       "the same for a `static` local of main: struct P g[3] = {{7, {1, 2, 3}}} (main holds program_start's values)",
       [=](Kit& k)
       {
           main_g3(k, "0", false, true);
       });
    lc("e1e2e_gsarr3_static_unreached_wrong_fail", "the `static` local form: the check that g[2].a[1] is 1 fails",
       [=](Kit& k)
       {
           k.fail("check", main_g3(k, "1", false, true)->loc);
       });
    // struct Q { int n; bool a[2]; } g[2] with every value unknown (C++ external linkage): main reads g[1].a[1] into a bool local; check(false) after it must fail; check(g[1].a[1]) is not provable
    const auto bool_leaf = [=](Kit& k, bool prove_value) -> const Stmt*
    {
        StructType* Q {k.strukt("Q", {{"n", k.i32}, {"a", k.prog.array_of(k.bl, 2U)}})};
        Global* g {k.prog.add_global("g", k.prog.array_of(k.st(Q), 2U), k.at(), nullptr)};
        for (int e {0}; e < 2; ++e)
        {
            GlobalLeaf el;
            el.kind = GlobalInit::Unknown;
            el.loc = k.at();
            GlobalLeaf ln;
            ln.kind = GlobalInit::Unknown;
            ln.loc = k.at();
            el.leaves.push_back(ln);
            GlobalLeaf la;
            la.kind = GlobalInit::Unknown;
            la.rest = ArrayRest::Unknown;
            la.loc = k.at();
            el.leaves.push_back(la);
            g->elems.push_back(el);
        }
        g->rest = ArrayRest::Unknown;
        Function* m {k.fn("main", k.i32)};
        Var* b {k.local(m, "b", k.bl)};
        const Expr* rd {k.deref(
            k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "1"), k.at()), 1, k.at()),
                          N(k, "1"), k.at()))};
        const Stmt* c {k.chk(prove_value ? rd : k.no())};
        m->body = k.blkc({k.decl(b, rd), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_gsarr_bool_leaf_unknown_vacuity_fail",
       "struct Q { int n; bool a[2]; } g[2] with every value unknown: main reads g[1].a[1] (no range fact for a bool) and a check(false) after it is reached and fails",
       [=](Kit& k)
       {
           k.fail("check", bool_leaf(k, false)->loc);
       });
    lc("e1e2e_gsarr_bool_leaf_unknown_value_fail", "the same: check(g[1].a[1]) is not provable for an unknown value",
       [=](Kit& k)
       {
           k.fail("check", bool_leaf(k, true)->loc);
       });
}

// ---- Part E, slice 3d (record 0064 item 12): the element copy `v[i] = v[j]` of an element with an array member of scalars: the cell leaf by a store, the array leaf by `copy_arr_<t>` under the guard `d != s` only where the two places can be one object ----
void define_part_e_struct_array_leaf_copy_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    auto arr = [](Kit& k, Function* f, StructType* P, unsigned n, bool zero)
    {
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), n), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        return v;
    };
    auto arr2 = [](Kit& k, Function* f, const char* name, StructType* P, unsigned n, bool zero)
    {
        Var* v {f->new_local(name, k.prog.array_of(k.st(P), n), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        return v;
    };
    // the element `base[i]` (base: the address of an array object or a pointer), its cell n, its array member a and the cell a[j]
    auto el = [](Kit& k, const Expr* base, const Expr* i)
    {
        return k.b.elem_addr(base, i, k.at());
    };
    auto nplace = [=](Kit& k, const Expr* base, const Expr* i)
    {
        return k.b.member_addr(el(k, base, i), 0, k.at());
    };
    auto amem = [=](Kit& k, const Expr* base, const Expr* i)
    {
        return k.b.member_addr(el(k, base, i), 1, k.at());
    };
    auto aplace = [=](Kit& k, const Expr* base, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(amem(k, base, i), j, k.at());
    };
    auto aread = [=](Kit& k, const Expr* base, const Expr* i, const char* j)
    {
        return k.deref(aplace(k, base, i, N(k, j)));
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    // the element copy `dst[di] = src[sj]`: the cell by a store, the array leaf by ArrayCopy (what the encoder emits); returns the ArrayCopy
    auto copy_elem = [=](Kit& k, const Expr* dst, const Expr* di, const Expr* src, const Expr* sj,
                         std::vector<const Stmt*>& body) -> const Stmt*
    {
        body.push_back(k.store(nplace(k, dst, di), k.deref(nplace(k, src, sj))));
        const Stmt* c {k.b.array_copy(amem(k, dst, di), amem(k, src, sj), k.at())};
        body.push_back(c);
        return c;
    };
    // write every leaf of v[e]: n = nv, a = {1, 2, <last>} (a[2] not written when `last` is null)
    auto fill1 = [=](Kit& k, Var* v, const char* e, const char* nv, const char* last, std::vector<const Stmt*>& body)
    {
        body.push_back(k.store(nplace(k, k.addr(v), N(k, e)), N(k, nv)));
        body.push_back(k.store(aplace(k, k.addr(v), N(k, e), N(k, "0")), N(k, "1")));
        body.push_back(k.store(aplace(k, k.addr(v), N(k, e), N(k, "1")), N(k, "2")));
        if (last != nullptr)
        {
            body.push_back(k.store(aplace(k, k.addr(v), N(k, e), N(k, "2")), N(k, last)));
        }
    };
    // void f(void) { struct P v[2]; v[0] = {3, {1, 2, 3}}; v[1] = v[0]; check(v[1].n == 3 && v[1].a[2] == <want>); v[1].a[0] = 9; check(v[0].a[0] == 1 && v[1].a[0] == 9); }
    const auto distinct = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd)};
        Var* v {arr(k, f, P, 2U, false)};
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        fill1(k, v, "0", "3", "3", body);
        copy_elem(k, k.addr(v), N(k, "1"), k.addr(v), N(k, "0"), body);
        const Stmt* c {k.chk(k.land(k.eq(k.deref(nplace(k, k.addr(v), N(k, "1"))), N(k, "3")),
                                    k.eq(aread(k, k.addr(v), N(k, "1"), "2"), N(k, want))))};
        body.push_back(c);
        body.push_back(k.store(aplace(k, k.addr(v), N(k, "1"), N(k, "0")), N(k, "9")));
        body.push_back(k.chk(k.land(k.eq(aread(k, k.addr(v), N(k, "0"), "0"), N(k, "1")),
                                    k.eq(aread(k, k.addr(v), N(k, "1"), "0"), N(k, "9")))));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lcv("e1e2e_sarr_copy_elements_distinct",
        "v[1] = v[0] for struct P { int n; int a[3]; }: the cell n by a store, the array member by copy_arr_int under the guard; the copy is independent of its source",
        [=](Kit& k)
        {
            distinct(k, "3");
        });
    lc("e1e2e_sarr_copy_elements_wrong_fail", "the same: the check that v[1].a[2] is 4 fails (the source's cell is 3)",
       [=](Kit& k)
       {
           k.fail("check", distinct(k, "4"));
       });
    // v[0] = v[0] (an exact self-copy of a named array's element): does nothing
    lcv("e1e2e_sarr_copy_self_constant",
        "v[0] = v[0]: the same element by constant indexes, an exact self-copy: the cells are unchanged",
        [=](Kit& k)
        {
            StructType* P {mk_p(k)};
            Function* f {k.fn("f", k.vd)};
            Var* v {arr(k, f, P, 2U, false)};
            std::vector<const Stmt*> body {k.decl(v, nullptr)};
            fill1(k, v, "0", "3", "3", body);
            copy_elem(k, k.addr(v), N(k, "0"), k.addr(v), N(k, "0"), body);
            body.push_back(k.chk(k.land(k.eq(aread(k, k.addr(v), N(k, "0"), "1"), N(k, "2")),
                                        k.eq(k.deref(nplace(k, k.addr(v), N(k, "0"))), N(k, "3")))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // void f(int i, int j) requires(0 <= i <= 1 && 0 <= j <= 1) { struct P v[2] (zero filled); v[0].a[1] = 5; v[i] = v[j]; check(v[i].a[1] == <v[j].a[1] | 5>); }
    const auto symbolic = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "1"), k.range(k.F(f, 1), k.i32, "0", "1")));
        Var* v {arr(k, f, P, 2U, true)};
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        body.push_back(k.store(aplace(k, k.addr(v), N(k, "0"), N(k, "1")), N(k, "5")));
        copy_elem(k, k.addr(v), k.pv(f, 0), k.addr(v), k.pv(f, 1), body);
        const Expr* want {wrong ? N(k, "5") : aread(k, k.addr(v), k.pv(f, 1), "1")};
        const Stmt* c {k.chk(k.eq(aread(k, k.addr(v), k.pv(f, 0), "1"), want))};
        body.push_back(c);
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_sarr_copy_symbolic_indexes",
       "v[i] = v[j] for i and j in range (equal or not): afterwards v[i].a[1] == v[j].a[1] (the guard d != s makes the equal case a no-op)",
       [=](Kit& k)
       {
           symbolic(k, false);
       });
    lc("e1e2e_sarr_copy_symbolic_wrong_fail",
       "the same: check(v[i].a[1] == 5) fails for i == 0, j == 1 (v[1].a[1] is zero)",
       [=](Kit& k)
       {
           k.fail("check", symbolic(k, true));
       });
    // the cost note of audit round E3a: void f(int i, int j) requires(0 <= i <= 2 && 0 <= j <= 2) { struct P v[3] (zero filled); v[0].a[1] = 5; v[i] = v[j]; check(v[i].a[1] == <v[j].a[1] | 5>); }: three elements, both indexes symbolic
    const auto symbolic3 = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "2"), k.range(k.F(f, 1), k.i32, "0", "2")));
        Var* v {arr(k, f, P, 3U, true)};
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        body.push_back(k.store(aplace(k, k.addr(v), N(k, "0"), N(k, "1")), N(k, "5")));
        copy_elem(k, k.addr(v), k.pv(f, 0), k.addr(v), k.pv(f, 1), body);
        const Expr* want {wrong ? N(k, "5") : aread(k, k.addr(v), k.pv(f, 1), "1")};
        const Stmt* c {k.chk(k.eq(aread(k, k.addr(v), k.pv(f, 0), "1"), want))};
        body.push_back(c);
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_sarr_copy_symbolic_three_elements",
       "v[i] = v[j] over three elements with both indexes symbolic: afterwards v[i].a[1] == v[j].a[1]",
       [=](Kit& k)
       {
           symbolic3(k, false);
       });
    lc("e1e2e_sarr_copy_symbolic_three_elements_wrong_fail",
       "the same with a false check (v[i].a[1] == 5): it fails (for j != 0), and the verdict arrives",
       [=](Kit& k)
       {
           k.fail("check", symbolic3(k, true));
       });
    // struct P v[2], w[2]: w[0] written; v[1] = w[0]; check(v[1].a[2] == 3): two named arrays
    lcv("e1e2e_sarr_copy_between_two_arrays", "v[1] = w[0]: an element of one named array into an element of another",
        [=](Kit& k)
        {
            StructType* P {mk_p(k)};
            Function* f {k.fn("f", k.vd)};
            Var* v {arr2(k, f, "v", P, 2U, false)};
            Var* w {arr2(k, f, "w", P, 2U, false)};
            std::vector<const Stmt*> body {k.decl(v, nullptr), k.decl(w, nullptr)};
            fill1(k, w, "0", "3", "3", body);
            copy_elem(k, k.addr(v), N(k, "1"), k.addr(w), N(k, "0"), body);
            body.push_back(k.chk(k.land(k.eq(aread(k, k.addr(v), N(k, "1"), "2"), N(k, "3")),
                                        k.eq(k.deref(nplace(k, k.addr(v), N(k, "1"))), N(k, "3")))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // void f(int i, int j) requires(0 <= i <= 1 && 0 <= j <= 1) { struct P v[2], w[2] (zero filled); w[0].a[1] = 5; v[i] = w[j]; check(v[i].a[1] == w[j].a[1]); }: two named arrays at symbolic indexes: no guard
    lc("e1e2e_sarr_copy_between_two_arrays_symbolic",
       "v[i] = w[j] for i and j in range: the two places are elements of two named arrays (distinct blocks whatever the indexes), so the array copy is not guarded and Silicon proves the call's own row",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
           k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "1"), k.range(k.F(f, 1), k.i32, "0", "1")));
           Var* v {arr2(k, f, "v", P, 2U, true)};
           Var* w {arr2(k, f, "w", P, 2U, true)};
           std::vector<const Stmt*> body {k.decl(v, nullptr), k.decl(w, nullptr)};
           body.push_back(k.store(aplace(k, k.addr(w), N(k, "0"), N(k, "1")), N(k, "5")));
           copy_elem(k, k.addr(v), k.pv(f, 0), k.addr(w), k.pv(f, 1), body);
           body.push_back(k.chk(k.eq(aread(k, k.addr(v), k.pv(f, 0), "1"), aread(k, k.addr(w), k.pv(f, 1), "1"))));
           f->body = k.blkc(body);
           k.done(f);
       });
    // SELF-COPY of a named array's element with nothing written: struct P v[2]; v[1] = v[1];: the source's rows come before the relation's no-op
    lc("e1e2e_sarr_copy_self_unwritten_fail",
       "v[1] = v[1] with the element never written: a self-copy reports what a copy reports: uninitialized at the read of n (Silicon's one error: the array copy's own flag row follows the failed one)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, false)};
           const Expr* rd {k.deref(nplace(k, k.addr(v), N(k, "1")))};
           const Stmt* cell {k.store(nplace(k, k.addr(v), N(k, "1")), rd)};
           const Stmt* c {k.b.array_copy(amem(k, k.addr(v), N(k, "1")), amem(k, k.addr(v), N(k, "1")), k.at())};
           f->body = k.blkc({k.decl(v, nullptr), cell, c});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    // v[0].a[2] never written: v[1] = v[0] copies an element without a value
    lc("e1e2e_sarr_copy_unwritten_source_fail", "v[1] = v[0] with v[0].a[2] never written: uninitialized at the copy",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {arr(k, f, P, 2U, false)};
           std::vector<const Stmt*> body {k.decl(v, nullptr)};
           fill1(k, v, "0", "3", nullptr, body);
           const Stmt* c {copy_elem(k, k.addr(v), N(k, "1"), k.addr(v), N(k, "0"), body)};
           f->body = k.blkc(body);
           k.done(f);
           k.fail("uninitialized", c->loc);
       });
    // void cp(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && p[1].a[2] == p[0].a[<2 | 1>]) { p[1] = p[0]; }
    const auto range_copy = [=](Kit& k, bool wrong) -> Function*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("cp", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        auto RO = [&]()
        {
            return k.b.range_atom(AtomKind::OwnsRange, k.b.text(k.F(f, 0), "p"), k.b.text(k.F(f, 1), "n"), 0, k.at());
        };
        k.req(f, k.land(RO(), k.gt(k.F(f, 1), N(k, "1"))));
        const Expr* a1 {k.deref(aplace(k, k.F(f, 0), N(k, "1"), N(k, "2")))};
        const Expr* a0 {k.deref(aplace(k, k.F(f, 0), N(k, "0"), N(k, "2")))};
        const Expr* other {k.deref(aplace(k, k.F(f, 0), N(k, "0"), N(k, "1")))};
        k.ens(f, k.land(RO(), k.eq(a1, wrong ? other : a0)));
        std::vector<const Stmt*> body;
        copy_elem(k, k.pv(f, 0), N(k, "1"), k.pv(f, 0), N(k, "0"), body);
        f->body = k.blkc(body);
        k.done(f);
        return f;
    };
    lc("e1e2e_sarr_copy_in_a_range",
       "p[1] = p[0] under owns_range(p, n): the places are reached through a pointer, so the array copy is guarded; the postcondition reads the copied cell",
       [=](Kit& k)
       {
           range_copy(k, false);
       });
    lc("e1e2e_sarr_copy_in_a_range_wrong_fail",
       "the same ensuring p[1].a[2] == p[0].a[1]: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           k.fail("postcondition", range_copy(k, true)->end_loc);
       });
    // void cp(struct P *p, struct P *q) requires(owns(*p) && reads(*q)) ensures(owns(*p) && reads(*q) && p->a[2] == q->a[2]) { *p = *q; }  is Part C2/D's (depth 1)
    // void g(struct P *p) requires(owns(*p)) ensures(owns(*p) && p->a[1] == 5) { struct P v[2] (zero filled); v[1].a[1] = 5; *p = v[1]; }: a pointer place from an element
    const auto from_element = [=](Kit& k, const char* post) -> Function*
    {
        StructType* P {mk_p(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(P))}})};
        auto own = [&]()
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at());
        };
        k.req(g, own());
        k.ens(g, k.land(own(), k.eq(k.deref(k.b.elem_addr(k.mem(k.F(g, 0), 1), N(k, "1"), k.at())), N(k, post))));
        Var* v {arr(k, g, P, 2U, true)};
        std::vector<const Stmt*> body {k.decl(v, nullptr)};
        body.push_back(k.store(aplace(k, k.addr(v), N(k, "1"), N(k, "1")), N(k, "5")));
        // *p = v[1]: the cell n by a store, the array member by ArrayCopy
        body.push_back(k.store(k.mem(k.pv(g, 0), 0), k.deref(nplace(k, k.addr(v), N(k, "1")))));
        body.push_back(k.b.array_copy(k.mem(k.pv(g, 0), 1), amem(k, k.addr(v), N(k, "1")), k.at()));
        g->body = k.blkc(body);
        k.done(g);
        return g;
    };
    lc("e1e2e_sarr_copy_element_into_a_pointer_place",
       "*p = v[1] under owns(*p): the destination is reached through a pointer (guarded), the source is an element of a named array",
       [=](Kit& k)
       {
           from_element(k, "5");
       });
    lc("e1e2e_sarr_copy_element_into_a_pointer_place_wrong_fail",
       "the same ensuring p->a[1] == 6: the postcondition fails",
       [=](Kit& k)
       {
           k.fail("postcondition", from_element(k, "6")->end_loc);
       });
}

// ---- Part E, slice 3d-m (the coordinator's addition; record 0064 item 12): the element copy of an element of an array MEMBER of structures, `t.v[i] = t.v[j]`, `p->v[i] = p->v[j]`, `gt.v[1] = gt.v[0]`, `t.v[0] = u.v[0]`: the cell n by a store, the array member a by `copy_arr_int` ----
void define_part_e_member_struct_array_leaf_copy_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_t = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"k", k.i32}});
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    // the element `base->v[idx]` (base: a pointer to the structure), its cell n, its array member a and the cell a[j]
    auto el = [](Kit& k, const Expr* base, const Expr* idx)
    {
        return k.b.elem_addr(k.mem(base, 0), idx, k.at());
    };
    auto nplace = [=](Kit& k, const Expr* base, const Expr* idx)
    {
        return k.b.member_addr(el(k, base, idx), 0, k.at());
    };
    auto amem = [=](Kit& k, const Expr* base, const Expr* idx)
    {
        return k.b.member_addr(el(k, base, idx), 1, k.at());
    };
    auto aplace = [=](Kit& k, const Expr* base, const Expr* idx, const char* j)
    {
        return k.b.elem_addr(amem(k, base, idx), N(k, j), k.at());
    };
    // the element copy `dst->v[di] = src->v[sj]`: the cell by a store, the array member by ArrayCopy; returns the ArrayCopy
    auto copy_elem = [=](Kit& k, const Expr* dst, const Expr* di, const Expr* src, const Expr* sj,
                         std::vector<const Stmt*>& body) -> const Stmt*
    {
        body.push_back(k.store(nplace(k, dst, di), k.deref(nplace(k, src, sj))));
        const Stmt* c {k.b.array_copy(amem(k, dst, di), amem(k, src, sj), k.at())};
        body.push_back(c);
        return c;
    };
    // struct T t; t.v[0] = {3, {1, 2, <last>}} (a[2] is not written when `last` is null), t.v[1] written when `both`
    auto fill = [=](Kit& k, Var* t, const char* last, bool both, std::vector<const Stmt*>& body)
    {
        body.push_back(k.decl(t, nullptr));
        for (const char* e : {"0", "1"})
        {
            if (e[0] == '1' && !both)
            {
                continue;
            }
            body.push_back(k.store(nplace(k, k.addr(t), N(k, e)), N(k, "3")));
            body.push_back(k.store(aplace(k, k.addr(t), N(k, e), "0"), N(k, "1")));
            body.push_back(k.store(aplace(k, k.addr(t), N(k, e), "1"), N(k, "2")));
            if (last != nullptr)
            {
                body.push_back(k.store(aplace(k, k.addr(t), N(k, e), "2"), N(k, last)));
            }
        }
        body.push_back(k.lstore(k.addr(t), 1, N(k, "7")));
    };
    auto aread = [=](Kit& k, const Expr* base, const Expr* idx, const char* j)
    {
        return k.deref(aplace(k, base, idx, j));
    };
    // struct T t; t.v[0] = {3,{1,2,3}}; t.v[1] = <written>; t.v[<di>] = t.v[<sj>]; check(t.v[<di>].a[2] == <want> && t.v[<di>].n == 3)
    const auto constant = [=](Kit& k, const char* di, const char* sj, const char* want) -> SrcLoc
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        std::vector<const Stmt*> body;
        fill(k, t, "3", true, body);
        body.push_back(k.store(aplace(k, k.addr(t), N(k, "1"), "2"), N(k, "9")));
        copy_elem(k, k.addr(t), N(k, di), k.addr(t), N(k, sj), body);
        const Stmt* c {k.chk(k.land(k.eq(aread(k, k.addr(t), N(k, di), "2"), N(k, want)),
                                    k.eq(k.deref(nplace(k, k.addr(t), N(k, di))), N(k, "3"))))};
        body.push_back(c);
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lcv("e1e2e_tsarr_copy_constant_distinct",
        "t.v[1] = t.v[0] (t.v[0].a[2] is 3, t.v[1].a[2] was 9): two different constant indexes of one array member, no guard: afterwards t.v[1].a[2] is 3",
        [=](Kit& k)
        {
            constant(k, "1", "0", "3");
        });
    lc("e1e2e_tsarr_copy_constant_distinct_wrong_fail",
       "the same: the check that t.v[1].a[2] is 9 fails (the copy replaced it)",
       [=](Kit& k)
       {
           k.fail("check", constant(k, "1", "0", "9"));
       });
    lcv("e1e2e_tsarr_copy_constant_self",
        "t.v[1] = t.v[1]: the same element by constant indexes (an exact self-copy): the source's rows are asserted, nothing is copied: t.v[1].a[2] is 9",
        [=](Kit& k)
        {
            constant(k, "1", "1", "9");
        });
    lc("e1e2e_tsarr_copy_constant_self_wrong_fail",
       "the same: the check that t.v[1].a[2] is 3 fails (the self-copy changes nothing)",
       [=](Kit& k)
       {
           k.fail("check", constant(k, "1", "1", "3"));
       });
    // void f(int i, int j) requires(0 <= i <= 1 && 0 <= j <= 1) { struct T t; t.v[0] = {3,{1,2,3}}; t.v[1] = {3,{1,2,4}}; t.v[i] = t.v[j]; check(t.v[i].a[2] == t.v[j].a[2] [wrong: == 3]); }
    const auto symbolic = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        k.req(f, k.land(k.range(k.F(f, 0), k.i32, "0", "1"), k.range(k.F(f, 1), k.i32, "0", "1")));
        Var* t {k.scell(f, "t", T)};
        std::vector<const Stmt*> body;
        fill(k, t, "3", true, body);
        body.push_back(k.store(aplace(k, k.addr(t), N(k, "1"), "2"), N(k, "4")));
        copy_elem(k, k.addr(t), k.pv(f, 0), k.addr(t), k.pv(f, 1), body);
        const Expr* want {wrong ? N(k, "3") : aread(k, k.addr(t), k.pv(f, 1), "2")};
        const Stmt* c {k.chk(k.eq(aread(k, k.addr(t), k.pv(f, 0), "2"), want))};
        body.push_back(c);
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lc("e1e2e_tsarr_copy_symbolic",
       "t.v[i] = t.v[j], i and j in range (equal or not): the array copy is under the guard d != s; afterwards t.v[i].a[2] == t.v[j].a[2]",
       [=](Kit& k)
       {
           symbolic(k, false);
       });
    lc("e1e2e_tsarr_copy_symbolic_wrong_fail",
       "the same: check(t.v[i].a[2] == 3) fails for i == 1, j == 1... or i == 0, j == 1 (the cell is 4)",
       [=](Kit& k)
       {
           k.fail("check", symbolic(k, true));
       });
    // struct T t, u; t.v[0] = u.v[0]: two named structures: no guard
    const auto two = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        Var* u {k.scell(f, "u", T)};
        std::vector<const Stmt*> body;
        fill(k, t, "9", false, body);
        body.push_back(k.decl(u, nullptr));
        body.push_back(k.store(nplace(k, k.addr(u), N(k, "0")), N(k, "5")));
        for (const char* j : {"0", "1", "2"})
        {
            body.push_back(k.store(aplace(k, k.addr(u), N(k, "0"), j), N(k, "6")));
        }
        copy_elem(k, k.addr(t), N(k, "0"), k.addr(u), N(k, "0"), body);
        const Stmt* c {k.chk(k.land(k.eq(aread(k, k.addr(t), N(k, "0"), "2"), N(k, want)),
                                    k.eq(k.deref(nplace(k, k.addr(t), N(k, "0"))), N(k, "5"))))};
        body.push_back(c);
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return c->loc;
    };
    lcv("e1e2e_tsarr_copy_between_two_structures",
        "t.v[0] = u.v[0] of two structure locals: two named objects, the array copy is not guarded; afterwards t.v[0].a[2] is 6",
        [=](Kit& k)
        {
            two(k, "6");
        });
    lc("e1e2e_tsarr_copy_between_two_structures_wrong_fail", "the same: the check that t.v[0].a[2] is 9 fails",
       [=](Kit& k)
       {
           k.fail("check", two(k, "9"));
       });
    // t.v[0].a[2] never written: t.v[1] = t.v[0] copies an element without a value
    lc("e1e2e_tsarr_copy_unwritten_source_fail",
       "t.v[1] = t.v[0] with t.v[0].a[2] never written: uninitialized at the copy",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, nullptr, false, body);
           const Stmt* c {copy_elem(k, k.addr(t), N(k, "1"), k.addr(t), N(k, "0"), body)};
           f->body = k.blkc(body);
           k.done(f);
           k.fail("uninitialized", c->loc);
       });
    // SELF-COPY: t.v[0] = t.v[0] with t.v[0].a[2] never written: the source's rows come before the relation's no-op, so the unwritten cell is reported at the copy
    lc("e1e2e_tsarr_copy_self_unwritten_cell_fail",
       "t.v[0] = t.v[0] with t.v[0].a[2] never written: a self-copy of an element asserts the source's flags before it does nothing: uninitialized at the copy",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, nullptr, false, body);
           const Stmt* c {copy_elem(k, k.addr(t), N(k, "0"), k.addr(t), N(k, "0"), body)};
           f->body = k.blkc(body);
           k.done(f);
           k.fail("uninitialized", c->loc);
       });
    // SELF-COPY of the cell n: t.v[0].n never written: the store of the load of n reports uninitialized at the load
    lc("e1e2e_tsarr_copy_self_unwritten_n_fail",
       "t.v[0] = t.v[0] with t.v[0].n never written: the cell copy reads n before the array copy: uninitialized at the read of n",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body {k.decl(t, nullptr)};
           for (const char* j : {"0", "1", "2"})
           {
               body.push_back(k.store(aplace(k, k.addr(t), N(k, "0"), j), N(k, "1")));
           }
           const Expr* rd {k.deref(nplace(k, k.addr(t), N(k, "0")))};
           body.push_back(k.store(nplace(k, k.addr(t), N(k, "0")), rd));
           body.push_back(k.b.array_copy(amem(k, k.addr(t), N(k, "0")), amem(k, k.addr(t), N(k, "0")), k.at()));
           f->body = k.blkc(body);
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    // void cp(struct T *p) requires(owns(*p)) ensures(owns(*p) && p->v[1].a[2] == p->v[0].a[<2 | 1>]) { p->v[1] = p->v[0]; }
    const auto ptr_copy = [=](Kit& k, bool wrong) -> Function*
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("cp", k.vd, {{"p", k.ptr(k.st(T))}})};
        auto own = [&]()
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
        };
        k.req(f, own());
        k.ens(f, k.land(own(),
                        k.eq(aread(k, k.F(f, 0), N(k, "1"), "2"), aread(k, k.F(f, 0), N(k, "0"), wrong ? "1" : "2"))));
        std::vector<const Stmt*> body;
        copy_elem(k, k.pv(f, 0), N(k, "1"), k.pv(f, 0), N(k, "0"), body);
        f->body = k.blkc(body);
        k.done(f);
        return f;
    };
    lc("e1e2e_tsarr_copy_through_a_pointer",
       "p->v[1] = p->v[0] under owns(*p): the places are reached through a pointer, two different constant indexes of one member (no guard needed: the chains differ); the postcondition reads the copied cell",
       [=](Kit& k)
       {
           ptr_copy(k, false);
       });
    lc("e1e2e_tsarr_copy_through_a_pointer_wrong_fail",
       "the same ensuring p->v[1].a[2] == p->v[0].a[1]: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           k.fail("postcondition", ptr_copy(k, true)->end_loc);
       });
    // void cp(struct T *p, int i, int j) requires(owns(*p) && 0 <= i <= 1 && 0 <= j <= 1) ensures(owns(*p) && p->v[1].a[2] == p->v[0].a[2] || ...) { p->v[i] = p->v[j]; }: symbolic indexes through a pointer: guarded
    const auto ptr_symbolic = [=](Kit& k, bool wrong) -> Function*
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("cp", k.vd, {{"p", k.ptr(k.st(T))}, {"i", k.i32}, {"j", k.i32}})};
        auto own = [&]()
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
        };
        k.req(f, k.land(own(), k.land(k.range(k.F(f, 1), k.i32, "0", "1"), k.range(k.F(f, 2), k.i32, "0", "1"))));
        // the postcondition reads p->v[1].a[2] against p->v[0].a[2] for i == 1 and j == 0; written with the indexes fixed so the contract reads are the supported forms
        k.ens(f, k.land(own(), k.implies(k.land(k.eq(k.F(f, 1), N(k, "1")), k.eq(k.F(f, 2), N(k, "0"))),
                                         k.eq(aread(k, k.F(f, 0), N(k, "1"), "2"),
                                              aread(k, k.F(f, 0), N(k, "0"), wrong ? "1" : "2")))));
        std::vector<const Stmt*> body;
        copy_elem(k, k.pv(f, 0), k.pv(f, 1), k.pv(f, 0), k.pv(f, 2), body);
        f->body = k.blkc(body);
        k.done(f);
        return f;
    };
    lc("e1e2e_tsarr_copy_through_a_pointer_symbolic",
       "p->v[i] = p->v[j] under owns(*p) for i and j in range: the places are reached through a pointer at indexes that are not constants: guarded by d != s; for i == 1 and j == 0 the postcondition reads the copied cell",
       [=](Kit& k)
       {
           ptr_symbolic(k, false);
       });
    lc("e1e2e_tsarr_copy_through_a_pointer_symbolic_wrong_fail",
       "the same ensuring p->v[1].a[2] == p->v[0].a[1] for i == 1 and j == 0: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           k.fail("postcondition", ptr_symbolic(k, true)->end_loc);
       });
    // struct T gt = {{{7, {1, 2, 3}}, {9, {4}}}, 5}: int main() { gt.v[1] = gt.v[0]; check(gt.v[1].n == 7 && gt.v[1].a[2] == <want> && gt.k == 5); }
    const auto gt_copy = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* T {mk_t(k)};
        Global* g {k.prog.add_global("gt", k.st(T), k.at(), nullptr)};
        GlobalLeaf lv;
        lv.kind = GlobalInit::Value;
        lv.rest = ArrayRest::Zero;
        lv.loc = k.at();
        const std::vector<std::pair<const char*, std::vector<const char*>>> els {{"7", {"1", "2", "3"}}, {"9", {"4"}}};
        for (const auto& [nv, cells] : els)
        {
            GlobalLeaf el;
            el.kind = GlobalInit::Value;
            el.loc = k.at();
            GlobalLeaf ln;
            ln.kind = GlobalInit::Value;
            ln.init = k.num(k.i32, nv);
            ln.value = nv;
            ln.loc = k.at();
            el.leaves.push_back(ln);
            GlobalLeaf la;
            la.kind = GlobalInit::Value;
            la.rest = ArrayRest::Zero;
            la.loc = k.at();
            for (const char* v : cells)
            {
                GlobalLeaf c;
                c.kind = GlobalInit::Value;
                c.value = v;
                c.init = k.num(k.i32, v);
                c.loc = k.at();
                la.elems.push_back(c);
            }
            el.leaves.push_back(la);
            lv.elems.push_back(el);
        }
        g->leaves.push_back(lv);
        GlobalLeaf lk;
        lk.kind = GlobalInit::Value;
        lk.init = k.num(k.i32, "5");
        lk.value = "5";
        lk.loc = k.at();
        g->leaves.push_back(lk);
        Function* m {k.fn("main", k.i32)};
        std::vector<const Stmt*> body;
        copy_elem(k, k.b.global_addr(g, k.at()), N(k, "1"), k.b.global_addr(g, k.at()), N(k, "0"), body);
        const Stmt* c {k.chk(k.land(k.land(k.eq(k.deref(nplace(k, k.b.global_addr(g, k.at()), N(k, "1"))), N(k, "7")),
                                           k.eq(aread(k, k.b.global_addr(g, k.at()), N(k, "1"), "2"), N(k, want))),
                                    k.eq(k.leaf(k.b.global_addr(g, k.at()), 1), N(k, "5"))))};
        body.push_back(c);
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return c->loc;
    };
    lc("e1e2e_gtsarr_copy_constant_distinct",
       "gt.v[1] = gt.v[0] of a global structure, in main (which holds program_start's values): the cell n by a store, the array member by copy_arr_int; afterwards gt.v[1].a[2] is 3, gt.v[1].n is 7, gt.k is 5",
       [=](Kit& k)
       {
           gt_copy(k, "3");
       });
    lc("e1e2e_gtsarr_copy_constant_distinct_wrong_fail",
       "the same: the check that gt.v[1].a[2] is 0 fails (the zero rest was replaced)",
       [=](Kit& k)
       {
           k.fail("check", gt_copy(k, "0"));
       });
}

// ---- Part E, slice 3e (record 0064 item 11, question 378): the Sequencer's key of a cell of an array member of an ELEMENT (`v[0].a[1]`, `g[0].a[1]`, `t.v[0].a[1]`, `p->v[0].a[1]`, `gt.v[0].a[1]`),
// and slice 3d's element copy of an element of an array MEMBER (`t.v[i] = t.v[j]`) ----
void define_part_e_struct_array_leaf_sequencer_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    auto mk_t = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"k", k.i32}});
    };
    // `base[i].a[j]` over the address of a named array of P or a pointer; `base->v[i].a[j]` over a structure pointer
    auto parr = [](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.member_addr(k.b.elem_addr(base, k.num(k.i32, i), k.at()), 1, k.at()), k.num(k.i32, j),
                             k.at());
    };
    auto tmem = [](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at()), 1, k.at()),
                             k.num(k.i32, j), k.at());
    };
    // X = X++ with the two places built by `mk`
    auto same = [](Kit& k, const std::function<const Expr*()>& mk) -> const Expr*
    {
        const Expr* inner {k.b.inc_dec_store_expr(mk(), true, true, k.at())};
        return k.b.store_expr(mk(), inner, k.at());
    };
    // a global array of P with every element zero (`owner`: a `static` local of that function)
    auto garr = [](Kit& k, StructType* P, const Function* owner) -> Global*
    {
        Global* g {k.prog.add_global("g", k.prog.array_of(k.st(P), 2U), k.at(), owner)};
        g->rest = ArrayRest::Zero;
        return g;
    };
    rc("e1e2e_rej_gsarr_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Global* g {garr(k, P, nullptr)};
           Function* f {k.fn("f", k.vd)};
           const Expr* o {same(k,
                               [&]
                               {
                                   return parr(k, k.b.global_addr(g, k.at()), "0", "1");
                               })};
           f->body = k.blkc({k.es(o)});
           k.done(f);
       });
    rc("e1e2e_rej_gsarr_static_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Global* g {garr(k, P, f)};
           const Expr* o {same(k,
                               [&]
                               {
                                   return parr(k, k.b.global_addr(g, k.at()), "0", "1");
                               })};
           f->body = k.blkc({k.es(o)});
           k.done(f);
       });
    lc("e1e2e_gsarr_seq_two_cells_of_one_member",
       "g[0].a[0] = g[0].a[1]++ of a global array: two cells of one member are two objects, no row fails",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Global* g {garr(k, P, nullptr)};
           Function* m {k.fn("main", k.i32)};
           const Expr* inner {
               k.b.inc_dec_store_expr(parr(k, k.b.global_addr(g, k.at()), "0", "1"), true, true, k.at())};
           const Expr* outer {k.b.store_expr(parr(k, k.b.global_addr(g, k.at()), "0", "0"), inner, k.at())};
           m->body = k.blkc({k.es(outer), k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    // a structure local: struct T t (zero filled)
    rc("e1e2e_rej_tsarr_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           const Expr* o {same(k,
                               [&]
                               {
                                   return tmem(k, k.addr(t), "0", "1");
                               })};
           f->body = k.blkc({k.decl(t, nullptr), k.b.array_zero(k.mem(k.addr(t), 0), k.at()), k.es(o)});
           k.done(f);
       });
    lc("e1e2e_tsarr_seq_two_elements_same_cell_index",
       "t.v[0].a[1] = t.v[1].a[1]++: the same cell index of two elements of an array member: two objects, no row fails",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           const Expr* inner {k.b.inc_dec_store_expr(tmem(k, k.addr(t), "1", "1"), true, true, k.at())};
           const Expr* outer {k.b.store_expr(tmem(k, k.addr(t), "0", "1"), inner, k.at())};
           f->body = k.blkc({k.decl(t, nullptr), k.b.array_zero(k.mem(k.addr(t), 0), k.at()), k.es(outer)});
           k.done(f);
       });
    lc("e1e2e_tsarr_seq_two_cells_of_one_member",
       "t.v[0].a[0] = t.v[0].a[1]++: two cells of one member are two objects, no row fails",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           const Expr* inner {k.b.inc_dec_store_expr(tmem(k, k.addr(t), "0", "1"), true, true, k.at())};
           const Expr* outer {k.b.store_expr(tmem(k, k.addr(t), "0", "0"), inner, k.at())};
           f->body = k.blkc({k.decl(t, nullptr), k.b.array_zero(k.mem(k.addr(t), 0), k.at()), k.es(outer)});
           k.done(f);
       });
    // through a pointer: void f(struct T *p) requires(owns(*p)) { p->v[0].a[1] = p->v[0].a[1]++; }
    rc("e1e2e_rej_tsarr_ptr_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at()));
           const Expr* o {same(k,
                               [&]
                               {
                                   return tmem(k, k.pv(f, 0), "0", "1");
                               })};
           f->body = k.blkc({k.es(o)});
           k.done(f);
       });
    // a structure global: struct T gt = {...zero...}
    const auto gt_global = [=](Kit& k) -> Global*
    {
        StructType* T {mk_t(k)};
        Global* g {k.prog.add_global("gt", k.st(T), k.at(), nullptr)};
        GlobalLeaf la;
        la.kind = GlobalInit::Value;
        la.rest = ArrayRest::Zero;
        la.loc = k.at();
        g->leaves.push_back(la);
        GlobalLeaf lk;
        lk.kind = GlobalInit::Value;
        lk.init = k.num(k.i32, "0");
        lk.value = "0";
        lk.loc = k.at();
        g->leaves.push_back(lk);
        return g;
    };
    rc("e1e2e_rej_gtsarr_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           Global* g {gt_global(k)};
           Function* f {k.fn("f", k.vd)};
           const Expr* o {same(k,
                               [&]
                               {
                                   return tmem(k, k.b.global_addr(g, k.at()), "0", "1");
                               })};
           f->body = k.blkc({k.es(o)});
           k.done(f);
       });
    // the Sequencer's struct-store rule meets the new keys: v[0] = (struct P){0, 0, v[0].a[1]++, 0}: the store writes every leaf of v[0], the value writes one of them
    rc("e1e2e_rej_sarr_seq_struct_store_writes_a_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(P), 2U), k.at())};
           f->mark_address_taken(v);
           v->zero_filled = true;
           const Expr* inc {k.b.inc_dec_store_expr(parr(k, k.addr(v), "0", "1"), true, true, k.at())};
           const Expr* value {k.b.aggregate(k.st(P), {N(k, "0"), N(k, "0"), inc, N(k, "0")}, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.store(k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), value)});
           k.done(f);
       });
    // a matrix member: struct U { int n; int m[2][3]; } u; u.m[1][1] = u.m[1][1]++ (rejected by the same Sequencer since slice 3b)
    rc("e1e2e_rej_mmember_seq_same_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* U {k.strukt("U", {{"n", k.i32}, {"m", e1e2e_matrix(k)}})};
           Function* f {k.fn("f", k.vd)};
           Var* u {k.scell(f, "u", U)};
           auto cell = [&]
           {
               return k.b.elem_addr(k.b.elem_addr(k.mem(k.addr(u), 1), N(k, "1"), k.at()), N(k, "1"), k.at());
           };
           const Expr* o {same(k, cell)};
           f->body = k.blkc({k.decl(u, nullptr), k.b.array_zero(k.mem(k.addr(u), 1), k.at()), k.es(o)});
           k.done(f);
       });
    // through a range: void f(struct P *p, int n) requires(owns_range(p, n) && n > 0) { p[0].a[1] = p[0].a[1]++; }: a pointer base has no key, lowering's path-precise row decides (unsequenced)
    lc("e1e2e_srange_seq_same_cell_fail",
       "p[0].a[1] = p[0].a[1]++ through a pointer parameter's range: no Sequencer key (a pointer base), lowering's path-precise row fails: unsequenced",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(k.land(k.b.range_atom(AtomKind::OwnsRange, k.b.text(k.F(f, 0), "p"),
                                                 k.b.text(k.F(f, 1), "n"), 0, k.at()),
                                  k.gt(k.F(f, 1), N(k, "0"))),
                           k.eq(k.deref(parr(k, k.F(f, 0), "0", "1")), N(k, "0"))));
           const Expr* inner {k.b.inc_dec_store_expr(parr(k, k.pv(f, 0), "0", "1"), true, true, k.at())};
           const Expr* outer {k.b.store_expr(parr(k, k.pv(f, 0), "0", "1"), inner, k.at())};
           f->body = k.blkc({k.es(outer)});
           k.done(f);
           k.fail_only("unsequenced", outer->loc, 1U, 0U);
       });
}

// ---- Part E, "3f" (the coordinator's addition after audit round E3a; cases only): program_start of a global structure whose NESTED by-value member holds a matrix, and a loop invariant that reads `g[e].a[e2]` of a global array of structures with an array member ----
void define_part_e_audit_e3a_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the global's rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    // (1) struct U { int n; int m[2][3]; }; struct V { struct U u; int k; }; static struct V gv = {{7, {{1, 2, 3}, {4, 5, 6}}}, 8};
    auto gv_global = [](Kit& k) -> Global*
    {
        StructType* U {k.strukt("U", {{"n", k.i32}, {"m", e1e2e_matrix(k)}})};
        StructType* V {k.strukt("V", {{"u", k.st(U)}, {"k", k.i32}})};
        Global* g {k.prog.add_global("gv", k.st(V), k.at(), nullptr)};
        auto scalar = [&](const char* v)
        {
            GlobalLeaf l;
            l.kind = GlobalInit::Value;
            l.init = k.num(k.i32, v);
            l.value = v;
            l.loc = k.at();
            return l;
        };
        g->leaves.push_back(scalar("7"));
        GlobalLeaf lm;
        lm.kind = GlobalInit::Value;
        lm.rest = ArrayRest::Zero;
        lm.loc = k.at();
        for (const std::vector<const char*>& r :
             std::vector<std::vector<const char*>> {{"1", "2", "3"}, {"4", "5", "6"}})
        {
            GlobalLeaf row;
            row.kind = GlobalInit::Value;
            row.rest = ArrayRest::Zero;
            row.loc = k.at();
            for (const char* v : r)
            {
                row.elems.push_back(scalar(v));
            }
            lm.elems.push_back(row);
        }
        g->leaves.push_back(lm);
        g->leaves.push_back(scalar("8"));
        return g;
    };
    auto gvcell = [=](Kit& k, const Global* g, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(k.mem(k.b.global_addr(g, k.at()), 0), 1), N(k, i), k.at()), N(k, j),
                             k.at());
    };
    // int main(void) { check(gv.u.n == 7 && gv.u.m[0][1] == 2 && gv.u.m[1][2] == <want> && gv.k == 8); return 0; }
    const auto gv_main = [=](Kit& k, const char* want) -> const Stmt*
    {
        Global* g {gv_global(k)};
        Function* m {k.fn("main", k.i32)};
        const Stmt* c {k.chk(k.land(k.land(k.eq(k.deref(k.mem(k.mem(k.b.global_addr(g, k.at()), 0), 0)), N(k, "7")),
                                           k.eq(k.deref(gvcell(k, g, "0", "1")), N(k, "2"))),
                                    k.land(k.eq(k.deref(gvcell(k, g, "1", "2")), N(k, want)),
                                           k.eq(k.deref(k.mem(k.b.global_addr(g, k.at()), 1)), N(k, "8")))))};
        std::vector<const Stmt*> body {c};
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lcv("e1e2e_gvnested_main_read",
        "static struct V gv = {{7, {{1, 2, 3}, {4, 5, 6}}}, 8} with struct V { struct U u; int k; }: main reads the nested member's n, two cells of its matrix and k",
        [=](Kit& k)
        {
            gv_main(k, "6");
        });
    lc("e1e2e_gvnested_main_read_wrong_fail", "the same: the check that gv.u.m[1][2] is 5 fails",
       [=](Kit& k)
       {
           k.fail("check", gv_main(k, "5")->loc);
       });
    // (2) struct P { int n; int a[3]; } g[2] = {{7, {1, 2, 3}}, {9, {4}}}: a loop invariant that reads g[1].a[2]
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 3U)}});
    };
    auto g2 = [=](Kit& k, StructType* P) -> Global*
    {
        Global* g {k.prog.add_global("g", k.prog.array_of(k.st(P), 2U), k.at(), nullptr)};
        const std::vector<std::pair<const char*, std::vector<const char*>>> els {{"7", {"1", "2", "3"}}, {"9", {"4"}}};
        for (const auto& [nv, cells] : els)
        {
            GlobalLeaf el;
            el.kind = GlobalInit::Value;
            el.loc = k.at();
            GlobalLeaf ln;
            ln.kind = GlobalInit::Value;
            ln.init = k.num(k.i32, nv);
            ln.value = nv;
            ln.loc = k.at();
            el.leaves.push_back(ln);
            GlobalLeaf la;
            la.kind = GlobalInit::Value;
            la.rest = ArrayRest::Zero;
            la.loc = k.at();
            for (const char* v : cells)
            {
                GlobalLeaf c;
                c.kind = GlobalInit::Value;
                c.value = v;
                c.init = k.num(k.i32, v);
                c.loc = k.at();
                la.elems.push_back(c);
            }
            el.leaves.push_back(la);
            g->elems.push_back(el);
        }
        g->rest = ArrayRest::Zero;
        return g;
    };
    auto gplace = [](Kit& k, const Global* g, const Expr* i, const Expr* j)
    {
        return k.b.elem_addr(k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), i, k.at()), 1, k.at()), j,
                             k.at());
    };
    // void f(void) requires(owns(g)) ensures(owns(g)) { g[1].a[2] = 0; int i = 0; while (i < 3) inv 0 <= i <= 3 && g[1].a[2] == 0 decreases 3 - i { [g[1].a[i] = 1;] i++; } }   int main() { f(); return 0; }
    const auto loop_f = [=](Kit& k, bool writes) -> const Clause
    {
        StructType* P {mk_p(k)};
        Global* g {g2(k, P)};
        auto atom = [&]()
        {
            return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
        };
        Function* f {k.fn("f", k.vd)};
        k.req(f, atom());
        k.ens(f, atom());
        Var* i {k.local(f, "i", k.i32)};
        const Clause keep {k.cl(k.eq(k.deref(gplace(k, g, N(k, "1"), N(k, "2"))), N(k, "0")))};
        std::vector<const Stmt*> lb;
        if (writes)
        {
            lb.push_back(k.store(gplace(k, g, N(k, "1"), k.v(i)), N(k, "1")));
        }
        lb.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")),
                              {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "3")))), keep},
                              {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(lb))};
        f->body = k.blkc({k.store(gplace(k, g, N(k, "1"), N(k, "2")), N(k, "0")), k.decl(i, N(k, "0")), lp});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return keep;
    };
    lc("e1e2e_gsarr_loop_invariant_owns_reads",
       "f holds owns(g): g[1].a[2] = 0, then a loop that only reads, its invariant reads g[1].a[2] == 0: the read is framed by the whole-array atom and carried by the loop's place",
       [=](Kit& k)
       {
           loop_f(k, false);
       });
    lc("e1e2e_gsarr_loop_invariant_owns_write_fail",
       "the same loop writing g[1].a[i] = 1: a store reaches the member's place, so g[1].a[2] == 0 is not kept (the invariant fails at the back edge)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", loop_f(k, true).loc);
       });
}

// ---- decision 0064 item 12 (Part E, slice 4a): structure VALUES of depth 2: the value view IS vir::value_cells ----
// struct U { int a; int m[2][2]; int b[2]; int c; }: the cells of a value, in the one enumeration: a, m[0][0], m[0][1], m[1][0], m[1][1], b[0], b[1], c (8 cells; 6 reached through an index step)
void define_part_e_deep_value_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mkU = [](Kit& k) -> StructType*
    {
        return k.strukt("U", {{"a", k.i32},
                              {"m", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)},
                              {"b", k.prog.array_of(k.i32, 2U)},
                              {"c", k.i32}});
    };
    auto mplace = [](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(base, 1), k.num(k.i32, i), k.at()), k.num(k.i32, j), k.at());
    };
    auto bplace = [](Kit& k, const Expr* base, const char* i)
    {
        return k.b.elem_addr(k.mem(base, 2), k.num(k.i32, i), k.at());
    };
    // the places of the eight cells of the object at `base`, in the order of the one enumeration
    auto places = [=](Kit& k, const Expr* base)
    {
        return std::vector<const Expr*> {k.mem(base, 0),
                                         mplace(k, base, "0", "0"),
                                         mplace(k, base, "0", "1"),
                                         mplace(k, base, "1", "0"),
                                         mplace(k, base, "1", "1"),
                                         bplace(k, base, "0"),
                                         bplace(k, base, "1"),
                                         k.mem(base, 3)};
    };
    // the reads of the eight cells of the object `u`, in order
    auto reads_of = [=](Kit& k, const Var* u)
    {
        std::vector<const Expr*> args;
        for (const Expr* p : places(k, k.addr(u)))
        {
            args.push_back(k.deref(p));
        }
        return args;
    };
    // `u` declared, then its cells written with the values 1 to 8 (the cell `skip` left unwritten when it is >= 0)
    auto fill = [=](Kit& k, const Var* u, int skip, std::vector<const Stmt*>& body)
    {
        body.push_back(k.decl(u, nullptr));
        static const char* const vals[] {"1", "2", "3", "4", "5", "6", "7", "8"};
        const std::vector<const Expr*> ps {places(k, k.addr(u))};
        for (size_t i {0U}; i < ps.size(); ++i)
        {
            if (static_cast<int>(i) != skip)
            {
                body.push_back(k.store(ps[i], N(k, vals[i])));
            }
        }
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    // the contract of g and of mk about the cells BESIDE the matrix: v.a == 1, v.b[0] == 6, v.b[1] == 7, v.c == 8 (the matrix's cells are not readable: question 364)
    auto beside = [=](Kit& k, const Expr* v)
    {
        return k.land(k.land(k.eq_c(k.b.field(v, 0, k.at()), k.i32, "1"),
                             k.eq_c(k.b.elem_value(v, 2, N(k, "0"), k.at()), k.i32, "6")),
                      k.land(k.eq_c(k.b.elem_value(v, 2, N(k, "1"), k.at()), k.i32, "7"),
                             k.eq_c(k.b.field(v, 3, k.at()), k.i32, "8")));
    };
    // void g(struct U v) requires(beside(v)) trusted;   void f(void) { struct U u; <cells 1..8>; g(<the value of u, `shift` cells late>); }
    auto call_g = [=](Kit& k, int shift, int skip) -> const Expr*
    {
        StructType* S {mkU(k)};
        Function* g {k.fn("g", k.vd, {{"v", k.st(S)}})};
        g->mark_address_taken(g->param_var(0));
        k.req(g, beside(k, k.F(g, 0)));
        k.trusted(g);
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        std::vector<const Stmt*> body;
        fill(k, u, skip, body);
        std::vector<const Expr*> args {reads_of(k, u)};
        if (shift != 0)
        {
            // the cells one place late: what a value view that disagrees about the matrix's width would pass
            args.erase(args.begin());
            args.push_back(args.back());
        }
        const Expr* call {k.call(g, {k.b.aggregate(k.st(S), args, k.at())})};
        body.push_back(k.es(call));
        tail(k, body);
        if (shift == 0 && skip < 0)
        {
            // the text pin of the order of the cells of a by-value call (record item 12, "Tested so"): the call's arguments are the object's eight cells in the one enumeration, and the callee's
            // formals are named by the steps of the same cells, in the same order
            k.text_checks.push_back(
                [](const std::string& vpr) -> std::string
                {
                    static const std::regex first {R"(f_g__[0-9]+\(loc\(member\((v_u__[0-9]+), 0\), 0\)\.v_int)"};
                    std::smatch m;
                    if (!std::regex_search(vpr, m, first))
                    {
                        return "the call of g was not found";
                    }
                    const std::string u {m[1].str()};
                    const std::string cell[] {"loc(member(" + u + ", 0), 0)",
                                              "loc(member(loc(member(" + u + ", 1), 0), 0), 0)",
                                              "loc(member(loc(member(" + u + ", 1), 0), 0), 1)",
                                              "loc(member(loc(member(" + u + ", 1), 1), 0), 0)",
                                              "loc(member(loc(member(" + u + ", 1), 1), 0), 1)",
                                              "loc(member(" + u + ", 2), 0)",
                                              "loc(member(" + u + ", 2), 1)",
                                              "loc(member(" + u + ", 3), 0)"};
                    const size_t at {static_cast<size_t>(m.position(0))};
                    const size_t eol {vpr.find('\n', at)};
                    const std::string line {vpr.substr(at, eol - at)};
                    size_t pos {0U};
                    for (const std::string& c : cell)
                    {
                        const size_t f {line.find(c + ".v_int", pos)};
                        if (f == std::string::npos)
                        {
                            return "the call's arguments are not the cells of u in the order of value_cells: missing " +
                                   c;
                        }
                        pos = f + c.size();
                    }
                    const size_t hdr {vpr.find("method f_g__")};
                    const std::string head {vpr.substr(hdr, vpr.find('\n', hdr) - hdr)};
                    size_t hp {0U};
                    for (const char* n : {"p_v__0_m0:", "p_v__0_m1_e0_e0:", "p_v__0_m1_e0_e1:", "p_v__0_m1_e1_e0:",
                                          "p_v__0_m1_e1_e1:", "p_v__0_m2_e0:", "p_v__0_m2_e1:", "p_v__0_m3:"})
                    {
                        const size_t f {head.find(n, hp)};
                        if (f == std::string::npos)
                        {
                            return std::string("the formals of g are not in the order of value_cells: missing ") + n;
                        }
                        hp = f + 1U;
                    }
                    return "";
                });
        }
        f->body = k.blkc(body);
        k.done(f);
        return skip >= 0 ? args[static_cast<size_t>(skip)] : call;
    };
    lcv("e1e2e_dvalue_call_by_value",
        "g(u) of a structure with a matrix member beside two arrays and two cells: the value is the 8 cells in order, g requires v.a == 1, v.b[0] == 6, v.b[1] == 7, v.c == 8 and the caller's cells are right",
        [=](Kit& k)
        {
            call_g(k, 0, -1);
        });
    lc("e1e2e_dvalue_call_shifted_fail",
       "the same with the caller's cells one place late: the callee's v.a is the caller's m[0][0] and the precondition fails",
       [=](Kit& k)
       {
           k.fail("precondition", call_g(k, 1, -1)->loc);
       });
    lc("e1e2e_dvalue_call_uninitialized_fail",
       "the same with the matrix's cell m[1][0] never written: the read of it for the value is uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", call_g(k, 0, 3)->loc);
       });
    lc("e1e2e_dvalue_call_uninitialized_beside_fail",
       "the same with the cell b[1] beside the matrix never written: uninitialized at its read",
       [=](Kit& k)
       {
           k.fail("uninitialized", call_g(k, 0, 6)->loc);
       });
    // struct U mk(void) ensures(beside(res)) { return (struct U){1, 2, 3, 4, 5, 6, 7, 8}; }   (`shift`: the cells one late)
    auto result_mk = [=](Kit& k, int shift) -> const Stmt*
    {
        StructType* S {mkU(k)};
        Function* mk {k.fn("mk", k.st(S))};
        k.ens(mk, beside(k, k.R(mk)));
        std::vector<const Expr*> args;
        static const char* const vals[] {"1", "2", "3", "4", "5", "6", "7", "8"};
        for (size_t i {0U}; i < 8U; ++i)
        {
            args.push_back(N(k, vals[std::min<size_t>(i + static_cast<size_t>(shift), 7U)]));
        }
        const Stmt* r {k.ret(k.b.aggregate(k.st(S), args, k.at()))};
        mk->body = k.blk({r});
        k.done(mk);
        return r;
    };
    lc("e1e2e_dvalue_result",
       "mk() returns (struct U){1, 2, ..., 8}: the result is the 8 cells in order, ensures res.a == 1, res.b[0] == 6, res.b[1] == 7, res.c == 8 holds",
       [=](Kit& k)
       {
           result_mk(k, 0);
       });
    lc("e1e2e_dvalue_result_shifted_fail",
       "the same returning the cells one place late: res.a is the matrix's first cell and the postcondition fails",
       [=](Kit& k)
       {
           k.fail("postcondition", result_mk(k, 1)->loc);
       });
    // int h(struct U v) ensures(res == v.<x>) { return <the cell of v's own copy>; }: the entry copy of the formals into the parameter's cells, by cell (a, b[0], b[1], c beside the matrix);
    // `wrong` makes h return the cell one place before the one the contract names
    auto param_cell = [=](Kit& k, int cell, int named, int wrong_cell) -> const Stmt*
    {
        StructType* S {mkU(k)};
        Function* h {k.fn("h", k.i32, {{"v", k.st(S)}})};
        h->mark_address_taken(h->param_var(0));
        const Expr* v {k.F(h, 0)};
        const Expr* want {
            cell == 0 ? k.b.field(v, 0, k.at())
                      : (cell == 1 ? k.b.elem_value(v, 2, N(k, "0"), k.at())
                                   : (cell == 2 ? k.b.elem_value(v, 2, N(k, "1"), k.at()) : k.b.field(v, 3, k.at())))};
        (void)named;
        k.ens(h, k.eq(k.R(h), want));
        const std::vector<const Expr*> reads {reads_of(k, h->param_var(0))};
        // reads: a, m00, m01, m10, m11, b0, b1, c -> the cell indexes of the beside cells are 0, 5, 6, 7
        static const size_t at_of[] {0U, 5U, 6U, 7U};
        const Stmt* r {k.ret(reads[at_of[wrong_cell]])};
        h->body = k.blk({r});
        k.done(h);
        return r;
    };
    for (int cell : {0, 1, 2, 3})
    {
        static const char* const names[] {"a", "b0", "b1", "c"};
        lc(std::string("e1e2e_dvalue_param_cell_") + names[cell],
           "a by-value parameter with a matrix member: the entry copy puts each formal in its own cell, so a read of the parameter's cell is the contract's v.<x> (a, b[0], b[1], c)",
           [=](Kit& k)
           {
               param_cell(k, cell, cell, cell);
           });
        lc(std::string("e1e2e_dvalue_param_cell_") + names[cell] + "_wrong_fail",
           "the same returning the next cell beside the matrix (cyclically) while the contract names v.<x>: the postcondition fails",
           [=](Kit& k)
           {
               k.fail("postcondition", param_cell(k, cell, cell, (cell + 1) % 4)->loc);
           });
    }
    // struct U id(struct U v) ensures(beside values of res and v agree) { return (struct U){the cells of v's own copy}; }   (`swap`: b[0] and b[1] exchanged)
    auto id_fn = [=](Kit& k, bool swap) -> const Stmt*
    {
        StructType* S {mkU(k)};
        Function* id {k.fn("id", k.st(S), {{"v", k.st(S)}})};
        id->mark_address_taken(id->param_var(0));
        const Expr* v {k.F(id, 0)};
        const Expr* r {k.R(id)};
        k.ens(id, k.land(k.land(k.eq(k.b.field(r, 0, k.at()), k.b.field(v, 0, k.at())),
                                k.eq(k.b.elem_value(r, 2, N(k, "0"), k.at()), k.b.elem_value(v, 2, N(k, "0"), k.at()))),
                         k.land(k.eq(k.b.elem_value(r, 2, N(k, "1"), k.at()), k.b.elem_value(v, 2, N(k, "1"), k.at())),
                                k.eq(k.b.field(r, 3, k.at()), k.b.field(v, 3, k.at())))));
        std::vector<const Expr*> args {reads_of(k, id->param_var(0))};
        if (swap)
        {
            std::swap(args[5], args[6]);
        }
        const Stmt* ret {k.ret(k.b.aggregate(k.st(S), args, k.at()))};
        id->body = k.blk({ret});
        k.done(id);
        return ret;
    };
    lc("e1e2e_dvalue_return_the_parameter",
       "id(v) returns the value of its own copy: the result's a, b[0], b[1], c are the argument's",
       [=](Kit& k)
       {
           id_fn(k, false);
       });
    lc("e1e2e_dvalue_return_the_parameter_swapped_fail",
       "the same returning b[0] and b[1] exchanged: the postcondition fails",
       [=](Kit& k)
       {
           k.fail("postcondition", id_fn(k, true)->loc);
       });
    // struct U mk(void) ensures(beside(res)) trusted;  void f(void) { struct U u; u = mk(); check(u.a == 1 && u.b[0] == 6 && u.b[1] == 7 && u.c == 8 && u.m[1][0] == u.m[1][0]); }
    // (the stored value is written cell by cell: the flags of the matrix's cells are set, their values unknown)
    auto store_result = [=](Kit& k, const char* b0) -> const Stmt*
    {
        StructType* S {mkU(k)};
        Function* mk {k.fn("mk", k.st(S))};
        k.ens(mk, beside(k, k.R(mk)));
        k.trusted(mk);
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        const std::vector<const Expr*> ps {places(k, k.addr(u))};
        const Expr* m10 {k.deref(mplace(k, k.addr(u), "1", "0"))};
        const Stmt* c {
            k.chk(k.land(k.land(k.land(k.eq_c(k.deref(ps[0]), k.i32, "1"), k.eq_c(k.deref(ps[5]), k.i32, b0)),
                                k.land(k.eq_c(k.deref(ps[6]), k.i32, "7"), k.eq_c(k.deref(ps[7]), k.i32, "8"))),
                         k.eq(m10, m10)))};
        f->body = k.blkc({k.decl(u, nullptr), k.store(k.addr(u), k.call(mk, {})), c});
        k.done(f);
        return c;
    };
    lc("e1e2e_dvalue_store_result",
       "u = mk(): the value's eight cells are stored, one by leaf place: u.a == 1, u.b == {6, 7}, u.c == 8 and every matrix cell is initialized",
       [=](Kit& k)
       {
           store_result(k, "6");
       });
    lc("e1e2e_dvalue_store_result_wrong_fail", "the same checking u.b[0] == 5: fails",
       [=](Kit& k)
       {
           k.fail("check", store_result(k, "5")->loc);
       });
    // ---- the second type of the record: a member array of structures whose element has an array leaf.  struct W { int a[2]; int n; }; struct T { struct W v[2]; int k; }: the cells of a value are
    // v[0].a[0], v[0].a[1], v[0].n, v[1].a[0], v[1].a[1], v[1].n, k (7 cells, 6 reached through an index step); a contract reads only k (t.v[i].n of a value is refused: record 0064 item 12) ----
    auto mkT = [](Kit& k) -> StructType*
    {
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 2U)}, {"n", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}, {"k", k.i32}});
    };
    // the places of the seven cells of the object at `base`, in the order of the one enumeration
    auto tplaces = [](Kit& k, const Expr* base)
    {
        std::vector<const Expr*> out;
        for (const char* i : {"0", "1"})
        {
            const Expr* el {k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at())};
            out.push_back(k.b.elem_addr(k.mem(el, 0), k.num(k.i32, "0"), k.at()));
            out.push_back(k.b.elem_addr(k.mem(el, 0), k.num(k.i32, "1"), k.at()));
            out.push_back(k.mem(el, 1));
        }
        out.push_back(k.mem(base, 1));
        return out;
    };
    // void g(struct T t) requires(t.k == 7) trusted;  void f(void) { struct T x; <cells 1..6, k = 7>; g(<the value of x>, `shift`: one cell late at the front: x0, x0, x1, ..., x5>); }
    auto call_t = [=](Kit& k, bool shift) -> const Expr*
    {
        StructType* S {mkT(k)};
        Function* g {k.fn("g", k.vd, {{"t", k.st(S)}})};
        g->mark_address_taken(g->param_var(0));
        k.req(g, k.eq_c(k.b.field(k.F(g, 0), 1, k.at()), k.i32, "7"));
        k.trusted(g);
        Function* f {k.fn("f", k.vd)};
        Var* x {k.scell(f, "x", S)};
        std::vector<const Stmt*> body {k.decl(x, nullptr)};
        static const char* const vals[] {"1", "2", "3", "4", "5", "6", "7"};
        const std::vector<const Expr*> ps {tplaces(k, k.addr(x))};
        for (size_t i {0U}; i < ps.size(); ++i)
        {
            body.push_back(k.store(ps[i], N(k, vals[i])));
        }
        std::vector<const Expr*> args;
        for (const Expr* p : tplaces(k, k.addr(x)))
        {
            args.push_back(k.deref(p));
        }
        if (shift)
        {
            args.insert(args.begin(), args.front());
            args.pop_back();
        }
        const Expr* call {k.call(g, {k.b.aggregate(k.st(S), args, k.at())})};
        body.push_back(k.es(call));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return call;
    };
    lcv("e1e2e_dvalue_nested_call_by_value",
        "g(x) of a structure whose member is an array of structures with an array leaf: the value is the 7 cells in order (v[0].a[0], v[0].a[1], v[0].n, v[1].a[0], v[1].a[1], v[1].n, k), g requires t.k == 7 and the caller's cells are right",
        [=](Kit& k)
        {
            call_t(k, false);
        });
    lc("e1e2e_dvalue_nested_call_shifted_fail",
       "the same with the caller's cells one place late (the first cell twice): the callee's t.k is the caller's v[1].n (6) and the precondition fails",
       [=](Kit& k)
       {
           k.fail("precondition", call_t(k, true)->loc);
       });
    // int h(struct T t) ensures(res == t.k) { return <the cell n of t.v[1] of the parameter's own copy, or its k>; }
    auto param_t = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S {mkT(k)};
        Function* h {k.fn("h", k.i32, {{"t", k.st(S)}})};
        h->mark_address_taken(h->param_var(0));
        k.ens(h, k.eq(k.R(h), k.b.field(k.F(h, 0), 1, k.at())));
        const std::vector<const Expr*> ps {tplaces(k, k.addr(h->param_var(0)))};
        const Stmt* r {k.ret(k.deref(ps[wrong ? 5U : 6U]))};
        h->body = k.blk({r});
        k.done(h);
        return r;
    };
    lc("e1e2e_dvalue_nested_param_cell",
       "h(t) returns the cell k of its own copy: the entry copy writes each of the seven formals into its own cell, so the read is the contract's t.k",
       [=](Kit& k)
       {
           param_t(k, false);
       });
    lc("e1e2e_dvalue_nested_param_cell_wrong_fail",
       "the same returning t.v[1].n (the cell before k): the postcondition fails",
       [=](Kit& k)
       {
           k.fail("postcondition", param_t(k, true)->loc);
       });
    // struct T id(struct T t) ensures(res.k == t.k) { return (struct T){the cells of t's own copy}; }   (`swap`: the last two cells exchanged)
    auto id_t = [=](Kit& k, bool swap) -> const Stmt*
    {
        StructType* S {mkT(k)};
        Function* id {k.fn("id", k.st(S), {{"t", k.st(S)}})};
        id->mark_address_taken(id->param_var(0));
        k.ens(id, k.eq(k.b.field(k.R(id), 1, k.at()), k.b.field(k.F(id, 0), 1, k.at())));
        std::vector<const Expr*> args;
        for (const Expr* p : tplaces(k, k.addr(id->param_var(0))))
        {
            args.push_back(k.deref(p));
        }
        if (swap)
        {
            std::swap(args[5], args[6]);
        }
        const Stmt* ret {k.ret(k.b.aggregate(k.st(S), args, k.at()))};
        id->body = k.blk({ret});
        k.done(id);
        return ret;
    };
    lc("e1e2e_dvalue_nested_return_the_parameter",
       "id(t) returns the value of its own copy: the result's k is the argument's",
       [=](Kit& k)
       {
           id_t(k, false);
       });
    lc("e1e2e_dvalue_nested_return_the_parameter_swapped_fail",
       "the same returning v[1].n and k exchanged: the postcondition res.k == t.k fails",
       [=](Kit& k)
       {
           k.fail("postcondition", id_t(k, true)->loc);
       });
    // the limit of 7 accepts exactly 7 cells reached through an index step: struct U7 { int a; int m[2][3]; int b[1]; int c; } passed by value (the 8-cell struct is the rejection case
    // e1e2e_rej_value_parameter_matrix_member_beyond_the_limit)
    lc("e1e2e_dvalue_seven_cells",
       "a structure with a matrix member of 6 cells and an array member of 1: 7 cells reached through an index step are accepted by value (the 8th is rejected): g(u) requires v.a == 1 && v.b[0] == 8 && v.c == 9",
       [=](Kit& k)
       {
           StructType* S {k.strukt("U7", {{"a", k.i32},
                                          {"m", k.prog.array_of(k.prog.array_of(k.i32, 3U), 2U)},
                                          {"b", k.prog.array_of(k.i32, 1U)},
                                          {"c", k.i32}})};
           Function* g {k.fn("g", k.vd, {{"v", k.st(S)}})};
           g->mark_address_taken(g->param_var(0));
           const Expr* v {k.F(g, 0)};
           k.req(g, k.land(k.land(k.eq_c(k.b.field(v, 0, k.at()), k.i32, "1"),
                                  k.eq_c(k.b.elem_value(v, 2, N(k, "0"), k.at()), k.i32, "8")),
                           k.eq_c(k.b.field(v, 3, k.at()), k.i32, "9")));
           k.trusted(g);
           Function* f {k.fn("f", k.vd)};
           std::vector<const Expr*> args;
           for (const char* c : {"1", "2", "3", "4", "5", "6", "7", "8", "9"})
           {
               args.push_back(N(k, c));
           }
           f->body = k.blk({k.es(k.call(g, {k.b.aggregate(k.st(S), args, k.at())}))});
           k.done(f);
       });
}

// ---- decision 0064 item 12 (Part E, slice 4b): the WHOLE-STRUCTURE COPY of a type of depth 2, in the statements the encoder traces (edg-backend-report-18, "4b: the encoder's statements"): every scalar
// member and matrix cell read into a temporary before the first store, the scalar members and the matrix cells stored from them, a scalar array member (b) by `copy_arr_int` LAST; the guard `d != s` only
// where the two places can be one object ----
void define_part_e_deep_copy_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mkU = [](Kit& k) -> StructType*
    {
        return k.strukt("U", {{"a", k.i32},
                              {"m", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)},
                              {"b", k.prog.array_of(k.i32, 2U)},
                              {"c", k.i32}});
    };
    auto mplace = [](Kit& k, const Expr* base, const char* i, const char* j)
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(base, 1), k.num(k.i32, i), k.at()), k.num(k.i32, j), k.at());
    };
    auto bplace = [](Kit& k, const Expr* base, const char* i)
    {
        return k.b.elem_addr(k.mem(base, 2), k.num(k.i32, i), k.at());
    };
    auto places = [=](Kit& k, const Expr* base)
    {
        return std::vector<const Expr*> {k.mem(base, 0),
                                         mplace(k, base, "0", "0"),
                                         mplace(k, base, "0", "1"),
                                         mplace(k, base, "1", "0"),
                                         mplace(k, base, "1", "1"),
                                         bplace(k, base, "0"),
                                         bplace(k, base, "1"),
                                         k.mem(base, 3)};
    };
    // `d = s` for struct objects at the base expressions `dst()` and `src()`, in the statements the encoder traces (build/m4.5e1e2d/probeV/trace, both modes): a and c read into temporaries (copy_0,
    // copy_3), the four matrix cells row major into temporaries (copy_agg_0 .. 3), a then c stored from their temporaries, the four matrix cells stored, and the array copy of b LAST;
    // `skip` (1 to 4, the matrix cell in cell order) is left out of the stores
    struct CopyOut
    {
        std::vector<const Stmt*> stmts;
        std::vector<const Expr*> reads; // the reads of the source: a, the four matrix cells, c
        const Stmt* acopy {nullptr};
    };
    auto copy_stmts = [=](Kit& k, Function* f, const std::function<const Expr*()>& dst,
                          const std::function<const Expr*()>& src, int skip)
    {
        CopyOut out;
        const std::vector<const Expr*> sp {places(k, src())};
        Var* ta {k.local(f, "copy_0", k.i32)};
        Var* tc {k.local(f, "copy_3", k.i32)};
        const Expr* read_a {k.deref(sp[0])};
        const Expr* read_c {k.deref(sp[7])};
        out.stmts.push_back(k.decl(ta, read_a));
        out.stmts.push_back(k.decl(tc, read_c));
        std::vector<Var*> tmp;
        std::vector<const Expr*> mreads;
        for (size_t c {1U}; c <= 4U; ++c)
        {
            Var* t {k.local(f, "copy_agg_" + std::to_string(c - 1U), k.i32)};
            tmp.push_back(t);
            const Expr* read {k.deref(sp[c])};
            mreads.push_back(read);
            out.stmts.push_back(k.decl(t, read));
        }
        out.reads.push_back(read_a);
        for (const Expr* r : mreads)
        {
            out.reads.push_back(r);
        }
        out.reads.push_back(read_c);
        const std::vector<const Expr*> dp {places(k, dst())};
        out.stmts.push_back(k.store(dp[0], k.v(ta)));
        out.stmts.push_back(k.store(dp[7], k.v(tc)));
        for (size_t c {1U}; c <= 4U; ++c)
        {
            if (static_cast<int>(c) != skip)
            {
                out.stmts.push_back(k.store(dp[c], k.v(tmp[c - 1U])));
            }
        }
        out.acopy = k.b.array_copy(k.mem(dst(), 2), k.mem(src(), 2), k.at());
        out.stmts.push_back(out.acopy);
        return out;
    };
    // the eight cells of the object `u` written with 1 .. 8 (the cell `skip`, 0 to 7, left unwritten)
    auto fill = [=](Kit& k, const Var* u, int skip, std::vector<const Stmt*>& body)
    {
        body.push_back(k.decl(u, nullptr));
        static const char* const vals[] {"1", "2", "3", "4", "5", "6", "7", "8"};
        const std::vector<const Expr*> ps {places(k, k.addr(u))};
        for (size_t i {0U}; i < ps.size(); ++i)
        {
            if (static_cast<int>(i) != skip)
            {
                body.push_back(k.store(ps[i], N(k, vals[i])));
            }
        }
    };
    // check(w.a == 1 && w.m[0][0] == 2 && ... && w.c == 8), the cell `bad` (0 to 7) compared with one more than it holds
    auto all_cells = [=](Kit& k, const Var* w, int bad) -> const Stmt*
    {
        static const char* const vals[] {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
        const std::vector<const Expr*> ps {places(k, k.addr(w))};
        const Expr* all {nullptr};
        for (size_t i {0U}; i < ps.size(); ++i)
        {
            const Expr* e {k.eq_c(k.deref(ps[i]), k.i32, static_cast<int>(i) == bad ? vals[i + 1U] : vals[i])};
            all = all == nullptr ? e : k.land(all, e);
        }
        return k.chk(all);
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    // void f(void) { struct U u, w; <u: 1..8>; w = u; check(every cell of w is the cell of u); w.m[0][0] = 9; check(u.m[0][0] == 2); }   (`bad`: the cell the check names wrongly)
    auto locals = [=](Kit& k, int bad, int skip = 0) -> const Stmt*
    {
        StructType* S {mkU(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* w {k.scell(f, "w", S)};
        std::vector<const Stmt*> body;
        fill(k, u, -1, body);
        body.push_back(k.decl(w, nullptr));
        if (skip != 0)
        {
            // the cell whose store is left out holds 0 (w is otherwise written by the copy only)
            body.push_back(k.store(places(k, k.addr(w))[static_cast<size_t>(skip)], N(k, "0")));
        }
        for (const Stmt* c : copy_stmts(
                                 k, f,
                                 [&]
                                 {
                                     return k.addr(w);
                                 },
                                 [&]
                                 {
                                     return k.addr(u);
                                 },
                                 skip)
                                 .stmts)
        {
            body.push_back(c);
        }
        const Stmt* chk {all_cells(k, w, bad)};
        body.push_back(chk);
        body.push_back(k.store(mplace(k, k.addr(w), "0", "0"), N(k, "9")));
        body.push_back(k.chk(k.eq_c(k.deref(mplace(k, k.addr(u), "0", "0")), k.i32, "2")));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return chk;
    };
    lcv("e1e2e_dcopy_locals",
        "w = u of a structure with a matrix member between two locals: every cell of u read into a temporary, then stored: w holds all eight cells in order, and w is independent of u",
        [=](Kit& k)
        {
            locals(k, -1);
        });
    for (int bad : {1, 4, 5})
    {
        lc("e1e2e_dcopy_locals_wrong_cell" + std::to_string(bad) + "_fail",
           "the same checking cell " + std::to_string(bad) + " of w against the next cell's value: the check fails",
           [=](Kit& k)
           {
               k.fail("check", locals(k, bad)->loc);
           });
    }
    // `d = s` with d and s the struct objects `x` and `y` (`x == y`: a self-copy): the source's cell `unwritten` (0 to 7) never written; returns the location the failure is at
    auto unwritten_copy = [=](Kit& k, bool self, int unwritten) -> SrcLoc
    {
        StructType* S {mkU(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* w {k.scell(f, "w", S)};
        std::vector<const Stmt*> body;
        fill(k, u, unwritten, body);
        if (!self)
        {
            body.push_back(k.decl(w, nullptr));
        }
        Var* dst {self ? u : w};
        const CopyOut co {copy_stmts(
            k, f,
            [&]
            {
                return k.addr(dst);
            },
            [&]
            {
                return k.addr(u);
            },
            0)};
        for (const Stmt* c : co.stmts)
        {
            body.push_back(c);
        }
        f->body = k.blkc(body);
        k.done(f);
        // a, a matrix cell and c are read into temporaries: the failure is at that read (co.reads is a, the four matrix cells, c); a cell of b at the array copy's source rows
        if (unwritten >= 0 && unwritten <= 4)
        {
            return co.reads[static_cast<size_t>(unwritten)]->loc;
        }
        return unwritten == 7 ? co.reads[5]->loc : co.acopy->loc;
    };
    lc("e1e2e_dcopy_unwritten_matrix_cell_fail",
       "w = u with u.m[1][0] never written: the read of that cell into its temporary is uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", unwritten_copy(k, false, 3));
       });
    lc("e1e2e_dcopy_unwritten_array_cell_fail",
       "w = u with u.b[1] never written: the array copy's source flags are not all set: uninitialized at the copy",
       [=](Kit& k)
       {
           k.fail("uninitialized", unwritten_copy(k, false, 6));
       });
    lc("e1e2e_dcopy_self_unwritten_matrix_cell_fail",
       "u = u with u.m[0][0] never written: the source's rows come before anything is stored, so the self-copy reports what a copy reports",
       [=](Kit& k)
       {
           k.fail("uninitialized", unwritten_copy(k, true, 1));
       });
    // void f(void) { struct U u; <1..8>; u = u; check(every cell of u is as it was); }
    lcv("e1e2e_dcopy_self",
        "u = u: every matrix cell is read into a temporary before the first store, so a self-copy rewrites each cell with its own value (the array copy of a place with itself does nothing)",
        [=](Kit& k)
        {
            StructType* S {mkU(k)};
            Function* f {k.fn("f", k.vd)};
            Var* u {k.scell(f, "u", S)};
            std::vector<const Stmt*> body;
            fill(k, u, -1, body);
            for (const Stmt* c : copy_stmts(
                                     k, f,
                                     [&]
                                     {
                                         return k.addr(u);
                                     },
                                     [&]
                                     {
                                         return k.addr(u);
                                     },
                                     0)
                                     .stmts)
            {
                body.push_back(c);
            }
            body.push_back(all_cells(k, u, -1));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // void g(struct U *p, struct U *q) requires(owns(*p) && reads(*q)) ensures(owns(*p) && reads(*q) && <p's cells == q's cells>) { *p = *q; }   (`skip`: a matrix cell left out of the stores)
    auto through_pointers = [=](Kit& k, int skip) -> SrcLoc
    {
        StructType* S {mkU(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.st(S))}})};
        auto atom = [&](AtomKind kind, int which)
        {
            return k.b.atom(kind, k.b.text(k.F(g, which), which == 0 ? "p" : "q"), which, k.at());
        };
        k.req(g, k.land(atom(AtomKind::Owns, 0), atom(AtomKind::Reads, 1)));
        const Expr* eqs {nullptr};
        const std::vector<const Expr*> pp {places(k, k.F(g, 0))};
        const std::vector<const Expr*> qp {places(k, k.F(g, 1))};
        for (size_t i {0U}; i < pp.size(); ++i)
        {
            if (skip != 0 && static_cast<int>(i) != skip)
            {
                continue; // the twin names only the cell whose store is left out (eight false-able conjuncts took 364 s)
            }
            const Expr* e {k.eq(k.deref(pp[i]), k.deref(qp[i]))};
            eqs = eqs == nullptr ? e : k.land(eqs, e);
        }
        k.ens(g, k.land(k.land(atom(AtomKind::Owns, 0), atom(AtomKind::Reads, 1)), eqs));
        const CopyOut co {copy_stmts(
            k, g,
            [&]
            {
                return k.pv(g, 0);
            },
            [&]
            {
                return k.pv(g, 1);
            },
            skip)};
        g->body = k.blk(co.stmts);
        k.done(g);
        return g->end_loc;
    };
    lc("e1e2e_dcopy_through_pointers",
       "*p = *q under owns(*p) and reads(*q): every cell of *q into a temporary, then stored through p (the array copy guarded where the places may be one object): p's cells equal q's in the postcondition",
       [=](Kit& k)
       {
           through_pointers(k, 0);
       });
    // (the false twin of the full structure's shape, *p = *q with the postcondition p->m[1][0] == q->m[1][0] + 1, took 428 s in lane il and Silicon reported a second error at the end of the
    // postcondition: it is not in the tree, see report 8; the two-pointer shape's failing twins are the struct U2 cases below)
    // struct U2 { int m[2][2]; int c; }: the copy as the encoder writes it (c and the four cells read into temporaries, c stored, then the four cells); `skip` (1 to 4): that matrix cell's store is left out; the postcondition names that cell (m[1][0]) of *p equal to *q's
    // (a twin whose postcondition says it differs ends in 116 s: not in the tree)
    auto u2_pointers = [=](Kit& k, int skip) -> SrcLoc
    {
        StructType* S {k.strukt("U2", {{"m", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)}, {"c", k.i32}})};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.st(S))}})};
        auto atom = [&](AtomKind kind, int which)
        {
            return k.b.atom(kind, k.b.text(k.F(g, which), which == 0 ? "p" : "q"), which, k.at());
        };
        auto cell = [&](const Expr* base, const char* i, const char* j)
        {
            return k.b.elem_addr(k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at()), k.num(k.i32, j), k.at());
        };
        k.req(g, k.land(atom(AtomKind::Owns, 0), atom(AtomKind::Reads, 1)));
        const Expr* e {k.eq(k.deref(cell(k.F(g, 0), "1", "0")), k.deref(cell(k.F(g, 1), "1", "0")))};
        k.ens(g, k.land(k.land(atom(AtomKind::Owns, 0), atom(AtomKind::Reads, 1)), e));
        std::vector<const Stmt*> body;
        Var* tc {k.local(g, "copy_1", k.i32)};
        body.push_back(k.decl(tc, k.deref(k.mem(k.pv(g, 1), 1))));
        std::vector<Var*> tmp;
        size_t n {0U};
        for (const char* i : {"0", "1"})
        {
            for (const char* j : {"0", "1"})
            {
                Var* t {k.local(g, "copy_agg_" + std::to_string(n++), k.i32)};
                tmp.push_back(t);
                body.push_back(k.decl(t, k.deref(cell(k.pv(g, 1), i, j))));
            }
        }
        body.push_back(k.store(k.mem(k.pv(g, 0), 1), k.v(tc)));
        n = 0U;
        for (const char* i : {"0", "1"})
        {
            for (const char* j : {"0", "1"})
            {
                if (static_cast<int>(n) + 1 != skip)
                {
                    body.push_back(k.store(cell(k.pv(g, 0), i, j), k.v(tmp[n])));
                }
                ++n;
            }
        }
        g->body = k.blk(body);
        k.done(g);
        return g->end_loc;
    };
    lc("e1e2e_dcopy_u2_through_pointers",
       "struct U2 { int m[2][2]; int c; }: *p = *q under owns(*p) and reads(*q): p->m[1][0] == q->m[1][0]",
       [=](Kit& k)
       {
           u2_pointers(k, 0);
       });
    lc("e1e2e_dcopy_u2_through_pointers_missing_store_fail",
       "the same with the store of p->m[1][0] left out and the postcondition p->m[1][0] == q->m[1][0]: it does not follow",
       [=](Kit& k)
       {
           k.fail("postcondition", u2_pointers(k, 3));
       });
    // (a twin of the same shape through pointers verifies its failure correctly but takes 361 s in lane il, one conjunct or eight: listed in report 7; the twin below is the locals' shape)
    lc("e1e2e_dcopy_locals_missing_store_fail",
       "the copy w = u with the store of w.m[1][0] left out (w.m[1][0] was 0): the check that every cell of w is u's cell fails",
       [=](Kit& k)
       {
           k.fail("check", locals(k, -1, 3)->loc);
       });
    // void g(struct U *p) requires(owns(*p)) ensures(owns(*p) && p->b[1] == old(p->b[1]) && p->m[1][0] == old(p->m[1][0])) { *p = *p; }  (`unguarded`: the contract names b[1] against 0)
    auto same_pointer = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* S {mkU(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(S))}})};
        auto atom = [&]
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at());
        };
        k.req(g, atom());
        const std::vector<const Expr*> fp {places(k, k.F(g, 0))};
        const Expr* b1 {k.deref(fp[6])};
        const Expr* m10 {k.deref(fp[3])};
        k.ens(g, k.land(atom(), k.land(k.eq(b1, wrong ? N(k, "0") : k.old(k.deref(places(k, k.F(g, 0))[6]))),
                                       k.eq(m10, k.old(k.deref(places(k, k.F(g, 0))[3]))))));
        const CopyOut co {copy_stmts(
            k, g,
            [&]
            {
                return k.pv(g, 0);
            },
            [&]
            {
                return k.pv(g, 0);
            },
            0)};
        g->body = k.blk(co.stmts);
        k.done(g);
        return g->end_loc;
    };
    lc("e1e2e_dcopy_through_the_same_pointer",
       "*p = *p under owns(*p): the places can be one object, so the array copy is guarded by d != s and the matrix cells are rewritten with their own values: b[1] and m[1][0] are as they were",
       [=](Kit& k)
       {
           same_pointer(k, false);
       });
    lc("e1e2e_dcopy_through_the_same_pointer_wrong_fail",
       "the same with the postcondition p->b[1] == 0: it does not follow",
       [=](Kit& k)
       {
           k.fail("postcondition", same_pointer(k, true));
       });
    // the Sequencer's key of a matrix cell in a struct Store (decision 0064 item 11): `u = (struct U){1, 2, 3, 4, <x>++, 6, 7, 8}` with x the cell u.m[1][1] (rejected: the value writes a cell of the
    // destination) or w.m[1][1] of another object (accepted: u.m[1][1] == 5 and w.m[1][1] == 6 after)
    auto seq_store = [=](Kit& k, bool other, const Stmt** chk_out) -> const Expr*
    {
        StructType* S {mkU(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* w {k.scell(f, "w", S)};
        const Expr* place {mplace(k, k.addr(other ? w : u), "1", "1")};
        const Expr* inc {k.b.inc_dec_store_expr(place, true, true, k.at())};
        const Expr* value {k.b.aggregate(
            k.st(S), {N(k, "1"), N(k, "2"), N(k, "3"), N(k, "4"), inc, N(k, "6"), N(k, "7"), N(k, "8")}, k.at())};
        std::vector<const Stmt*> body {k.decl(u, nullptr), k.decl(w, nullptr)};
        if (other)
        {
            body.push_back(k.store(mplace(k, k.addr(w), "1", "1"), N(k, "5")));
        }
        body.push_back(k.store(k.addr(u), value));
        if (chk_out != nullptr)
        {
            *chk_out = k.chk(k.land(k.eq_c(k.deref(mplace(k, k.addr(u), "1", "1")), k.i32, "5"),
                                    k.eq_c(k.deref(mplace(k, k.addr(w), "1", "1")), k.i32, "6")));
            body.push_back(*chk_out);
        }
        f->body = k.blkc(body);
        k.done(f);
        return inc;
    };
    rc("e1e2e_rej_dcopy_struct_store_writes_a_matrix_cell", "unsequenced modification of",
       [=](Kit& k)
       {
           seq_store(k, false, nullptr);
       });
    lc("e1e2e_dcopy_struct_store_writes_another_objects_cell",
       "u = (struct U){1, 2, 3, 4, w.m[1][1]++, 6, 7, 8}: the increment is of another object, so the store is sequenced: u.m[1][1] == 5 and w.m[1][1] == 6",
       [=](Kit& k)
       {
           const Stmt* c {nullptr};
           seq_store(k, true, &c);
       });
    // the same through a POINTER base: g(struct U *p, struct U *q) requires(owns(*p) && owns(*q)) { *p = (struct U){1, 2, 3, 4, <x>++, 6, 7, 8}; } with x the cell p->m[1][1] (the value writes a leaf of
    // the structure it is stored to: the path-precise rows reject it, decision 0064 item 11) or q->m[1][1] (another pointer: the two may be one object)
    auto seq_store_ptr = [=](Kit& k, bool other_pointer, bool post) -> SrcLoc
    {
        StructType* S {mkU(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.st(S))}})};
        auto atom = [&](int which)
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(g, which), which == 0 ? "p" : "q"), which, k.at());
        };
        k.req(g, k.land(k.land(atom(0), atom(1)), k.eq_c(k.deref(mplace(k, k.F(g, 1), "1", "1")), k.i32, "5")));
        if (post)
        {
            k.ens(g, k.land(k.land(atom(0), atom(1)),
                            k.land(k.eq_c(k.deref(mplace(k, k.F(g, 0), "1", "1")), k.i32, "5"),
                                   k.eq_c(k.deref(mplace(k, k.F(g, 1), "1", "1")), k.i32, "6"))));
        }
        const Expr* inc {
            k.b.inc_dec_store_expr(mplace(k, k.pv(g, other_pointer ? 1 : 0), "1", "1"), true, true, k.at())};
        const Expr* value {k.b.aggregate(
            k.st(S), {N(k, "1"), N(k, "2"), N(k, "3"), N(k, "4"), inc, N(k, "6"), N(k, "7"), N(k, "8")}, k.at())};
        g->body = k.blk({k.store(k.pv(g, 0), value)});
        k.done(g);
        return g->end_loc;
    };
    rc("e1e2e_rej_dcopy_struct_store_writes_a_matrix_cell_through_the_pointer", "unsequenced modification of",
       [=](Kit& k)
       {
           seq_store_ptr(k, false, false);
       });
    lc("e1e2e_dcopy_struct_store_writes_another_pointers_cell",
       "*p = (struct U){1, 2, 3, 4, q->m[1][1]++, 6, 7, 8} under owns(*p) && owns(*q) and q->m[1][1] == 5: the two atoms are distinct objects, so the store is sequenced: p->m[1][1] == 5 and q->m[1][1] == 6",
       [=](Kit& k)
       {
           seq_store_ptr(k, true, true);
       });
    // (the same with a wrong postcondition, q->m[1][1] == 5, verifies its failure correctly but took 357 s in lane il: not in the tree, as for `*p = *q` in report 7)
    // ---- the second type of the record, copied whole: struct W { int a[2]; int n; }; struct T { struct W v[2]; int k; }: k read into a temporary (copy_1), the six cells of v in value_cells order
    // (copy_agg_0 .. 5), k stored first, then the six cells, no array copy (edg-backend-report-18, "T: whole copy and values") ----
    auto mkT = [](Kit& k) -> StructType*
    {
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 2U)}, {"n", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}, {"k", k.i32}});
    };
    // the places of the seven cells of the object at `base`: the six cells of v, then k
    auto tplaces = [](Kit& k, const Expr* base)
    {
        std::vector<const Expr*> out;
        for (const char* i : {"0", "1"})
        {
            const Expr* el {k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at())};
            out.push_back(k.b.elem_addr(k.mem(el, 0), k.num(k.i32, "0"), k.at()));
            out.push_back(k.b.elem_addr(k.mem(el, 0), k.num(k.i32, "1"), k.at()));
            out.push_back(k.mem(el, 1));
        }
        out.push_back(k.mem(base, 1));
        return out;
    };
    // `d = s` as traced; `skip` (0 to 5, a cell of v) is left out of the stores
    auto tcopy_stmts = [=](Kit& k, Function* f, const std::function<const Expr*()>& dst,
                           const std::function<const Expr*()>& src, int skip)
    {
        std::vector<const Stmt*> out;
        const std::vector<const Expr*> sp {tplaces(k, src())};
        Var* tk {k.local(f, "copy_1", k.i32)};
        out.push_back(k.decl(tk, k.deref(sp[6])));
        std::vector<Var*> tmp;
        for (size_t c {0U}; c < 6U; ++c)
        {
            Var* t {k.local(f, "copy_agg_" + std::to_string(c), k.i32)};
            tmp.push_back(t);
            out.push_back(k.decl(t, k.deref(sp[c])));
        }
        const std::vector<const Expr*> dp {tplaces(k, dst())};
        out.push_back(k.store(dp[6], k.v(tk)));
        for (size_t c {0U}; c < 6U; ++c)
        {
            if (static_cast<int>(c) != skip)
            {
                out.push_back(k.store(dp[c], k.v(tmp[c])));
            }
        }
        return out;
    };
    // void f(void) { struct T u, w; <u: 1..7>; w = u; check(every cell of w is u's); w.v[0].a[0] = 9; check(u.v[0].a[0] == 1); }   (`bad`: the cell the check names wrongly; `skip`: a store left out,
    // w's cell holding 0 before; `self`: u = u and the check on u)
    auto tlocals = [=](Kit& k, int bad, int skip, bool self) -> const Stmt*
    {
        StructType* S {mkT(k)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* w {k.scell(f, "w", S)};
        std::vector<const Stmt*> body {k.decl(u, nullptr)};
        static const char* const vals[] {"1", "2", "3", "4", "5", "6", "7", "8"};
        const std::vector<const Expr*> ps {tplaces(k, k.addr(u))};
        for (size_t i {0U}; i < ps.size(); ++i)
        {
            body.push_back(k.store(ps[i], N(k, vals[i])));
        }
        Var* dst {self ? u : w};
        if (!self)
        {
            body.push_back(k.decl(w, nullptr));
            if (skip >= 0)
            {
                body.push_back(k.store(tplaces(k, k.addr(w))[static_cast<size_t>(skip)], N(k, "0")));
            }
        }
        for (const Stmt* c : tcopy_stmts(
                 k, f,
                 [&]
                 {
                     return k.addr(dst);
                 },
                 [&]
                 {
                     return k.addr(u);
                 },
                 skip))
        {
            body.push_back(c);
        }
        const std::vector<const Expr*> wp {tplaces(k, k.addr(dst))};
        const Expr* all {nullptr};
        for (size_t i {0U}; i < wp.size(); ++i)
        {
            const Expr* e {k.eq_c(k.deref(wp[i]), k.i32, static_cast<int>(i) == bad ? vals[i + 1U] : vals[i])};
            all = all == nullptr ? e : k.land(all, e);
        }
        const Stmt* chk {k.chk(all)};
        body.push_back(chk);
        if (!self)
        {
            body.push_back(k.store(tplaces(k, k.addr(w))[0], N(k, "9")));
            body.push_back(k.chk(k.eq_c(k.deref(tplaces(k, k.addr(u))[0]), k.i32, "1")));
        }
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return chk;
    };
    lcv("e1e2e_dcopy_nested_locals",
        "w = u of a structure with an array of structures with an array leaf: k and the six cells of v read into temporaries, k stored first, then the six cells: w holds all seven cells, and w is independent of u",
        [=](Kit& k)
        {
            tlocals(k, -1, -1, false);
        });
    for (int bad : {0, 5})
    {
        lc("e1e2e_dcopy_nested_locals_wrong_cell" + std::to_string(bad) + "_fail",
           "the same checking cell " + std::to_string(bad) + " of w against the next cell's value: the check fails",
           [=](Kit& k)
           {
               k.fail("check", tlocals(k, bad, -1, false)->loc);
           });
    }
    lc("e1e2e_dcopy_nested_locals_missing_store_fail",
       "the copy with the store of w.v[1].a[0] left out (it was 0): the check that every cell of w is u's fails",
       [=](Kit& k)
       {
           k.fail("check", tlocals(k, -1, 3, false)->loc);
       });
    lcv("e1e2e_dcopy_nested_self",
        "u = u of the same type: every cell is read into a temporary before the first store, so a self-copy rewrites each cell with its own value",
        [=](Kit& k)
        {
            tlocals(k, -1, -1, true);
        });
    lc("e1e2e_dcopy_nested_self_wrong_fail", "u = u and the check naming v[0].n against the next cell's value: fails",
       [=](Kit& k)
       {
           k.fail("check", tlocals(k, 2, -1, true)->loc);
       });
}

// ---- the CALL of a function whose contract has TWO `reads` atoms of a structure with a matrix member or a member array of structures (the second internal error of the first-member-matrix
// family: "an array has no cell fields of its own", the call rows' pairing of two reads atoms); shape 0 struct U { int n; int m[2][2]; }, 1 struct U2 { int m[2][2]; int c; },
// 2 struct T { struct W v[2]; int k; } with struct W { int a[2]; int n; } ----
void define_part_e_reads_reads_call_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mkS = [](Kit& k, int shape) -> StructType*
    {
        const Type mat {k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)};
        if (shape == 0)
        {
            return k.strukt("U", {{"n", k.i32}, {"m", mat}});
        }
        if (shape == 1)
        {
            return k.strukt("U2", {{"m", mat}, {"c", k.i32}});
        }
        StructType* W {k.strukt("W", {{"a", k.prog.array_of(k.i32, 2U)}, {"n", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(W), 2U)}, {"k", k.i32}});
    };
    // the places of every cell of the object at `base`, in declaration order, and the index among them of the scalar the contracts read
    auto cells = [](Kit& k, int shape, const Expr* base) -> std::vector<const Expr*>
    {
        std::vector<const Expr*> out;
        auto mcell = [&](int member, const char* i, const char* j)
        {
            return k.b.elem_addr(k.b.elem_addr(k.mem(base, member), k.num(k.i32, i), k.at()), k.num(k.i32, j), k.at());
        };
        if (shape == 0)
        {
            out.push_back(k.mem(base, 0));
            for (const char* i : {"0", "1"})
            {
                for (const char* j : {"0", "1"})
                {
                    out.push_back(mcell(1, i, j));
                }
            }
        }
        else if (shape == 1)
        {
            for (const char* i : {"0", "1"})
            {
                for (const char* j : {"0", "1"})
                {
                    out.push_back(mcell(0, i, j));
                }
            }
            out.push_back(k.mem(base, 1));
        }
        else
        {
            for (const char* i : {"0", "1"})
            {
                const Expr* el {k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at())};
                out.push_back(k.b.elem_addr(k.mem(el, 0), k.num(k.i32, "0"), k.at()));
                out.push_back(k.b.elem_addr(k.mem(el, 0), k.num(k.i32, "1"), k.at()));
                out.push_back(k.mem(el, 1));
            }
            out.push_back(k.mem(base, 1));
        }
        return out;
    };
    auto scalar_cell = [](int shape) -> size_t
    {
        return shape == 0 ? 0U : (shape == 1 ? 4U : 6U);
    };
    auto scalar_member = [](int shape) -> int
    {
        return shape == 0 ? 0 : 1;
    };
    // the stores of every cell of `x`: the scalar 5, the others 1, 2, ...
    auto fill = [=](Kit& k, int shape, const Var* x, std::vector<const Stmt*>& body, int skip = -1)
    {
        body.push_back(k.decl(x, nullptr));
        const std::vector<const Expr*> ps {cells(k, shape, k.addr(x))};
        for (size_t i {0U}; i < ps.size(); ++i)
        {
            if (static_cast<int>(i) == skip)
            {
                continue;
            }
            body.push_back(
                k.store(ps[i], i == scalar_cell(shape) ? N(k, "5") : k.num(k.i32, std::to_string(i + 1U).c_str())));
        }
    };
    // int g(struct S *p, struct S *q) requires(reads(*p) && reads(*q)) ensures(reads(*p) && reads(*q) && result == p-><scalar>);  declared, defined elsewhere
    auto make_g = [=](Kit& k, StructType* S, int shape) -> Function*
    {
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.st(S))}})};
        auto atom = [&](int which)
        {
            return k.b.atom(AtomKind::Reads, k.b.text(k.F(g, which), which == 0 ? "p" : "q"), which, k.at());
        };
        k.req(g, k.land(atom(0), atom(1)));
        k.ens(g,
              k.land(k.land(atom(0), atom(1)), k.eq(k.R(g), k.leaf(k.b.text(k.F(g, 0), "p"), scalar_member(shape)))));
        k.trusted(g);
        return g;
    };
    // void f(void) { struct S u; struct S v; <fill>; int r = g(&u, <&u | &v>); check(r == <5 | 6>); [u.<scalar> = 1; check(u.<scalar> == 1);] }
    auto caller = [=](Kit& k, int shape, bool same, bool wrong, bool write_after, int skip = -1,
                      const Expr** call_out = nullptr) -> const Stmt*
    {
        StructType* S {mkS(k, shape)};
        Function* g {make_g(k, S, shape)};
        Function* f {k.fn("f", k.vd)};
        Var* u {k.scell(f, "u", S)};
        Var* v {k.scell(f, "v", S)};
        Var* r {k.local(f, "r", k.i32)};
        std::vector<const Stmt*> body;
        fill(k, shape, u, body);
        if (!same)
        {
            fill(k, shape, v, body, skip);
        }
        const Expr* callx {k.call(g, {k.addr(u), same ? k.addr(u) : k.addr(v)})};
        if (call_out != nullptr)
        {
            *call_out = callx;
        }
        body.push_back(k.decl(r, callx));
        const Stmt* c {k.chk(k.eq(k.v(r), N(k, wrong ? "6" : "5")))};
        body.push_back(c);
        if (write_after)
        {
            // the caller lent a fraction of u twice and holds the rest: it can write the scalar and the check after sees it
            body.push_back(k.store(cells(k, shape, k.addr(u))[scalar_cell(shape)], N(k, "1")));
            body.push_back(k.chk(k.eq(k.deref(cells(k, shape, k.addr(u))[scalar_cell(shape)]), N(k, "1"))));
        }
        f->body = k.blkc(body);
        k.done(f);
        return c;
    };
    static const char* const names[] {"u", "u2", "t"};
    static const char* const docs[] {
        "struct U { int n; int m[2][2]; }", "struct U2 { int m[2][2]; int c; }",
        "struct T { struct W v[2]; int k; } (a member array of structures with an array member)"};
    for (int shape : {0, 1, 2})
    {
        const std::string n {std::string("e1e2e_rrcall_") + names[shape]};
        const std::string d {docs[shape]};
        lc(n + "_distinct", d + ": g(&u, &v) with reads(*p) && reads(*q): both lend their cells, r == u's scalar",
           [=](Kit& k)
           {
               caller(k, shape, false, false, false);
           });
        // (struct T's two cases that take two objects of seven cells each to a failing verdict, the wrong value and the unwritten cell, took 123 s and 145 s in lane il: not in the tree)
        if (shape != 2)
        {
            lc(n + "_distinct_wrong_fail", d + ": the same checking r == 6: the check fails",
               [=](Kit& k)
               {
                   k.fail("check", caller(k, shape, false, true, false)->loc);
               });
            // the cell `skip` of the second structure (a matrix cell, or a cell of a member array's element) never written: the call's flag row for the part's rows reports it
            lc(n + "_unwritten_matrix_cell_fail",
               d + ": g(&u, &v) with v's cell " + std::to_string(shape == 1 ? 2 : 3) +
                   " never written: uninitialized at the call",
               [=](Kit& k)
               {
                   const Expr* callx {nullptr};
                   caller(k, shape, false, false, false, shape == 1 ? 2 : 3, &callx);
                   // the rows of the call that name an initialized cell (the scalar's of both objects, the whole matrix's of both) are four; the one that fails is the row over the second
                   // object's part
                   k.fail_only("uninitialized", callx->loc, 4U, 3U);
               });
        }
        lc(n + "_alias",
           d + ": g(&u, &u): two reads of one object are allowed; the caller keeps what it lent and writes the scalar afterwards",
           [=](Kit& k)
           {
               caller(k, shape, true, false, true);
           });
        lc(n + "_alias_wrong_fail", d + ": g(&u, &u) checking r == 6: the check fails",
           [=](Kit& k)
           {
               k.fail("check", caller(k, shape, true, true, false)->loc);
           });
    }
    // int h(struct S *p) requires(reads(*p)) ensures(reads(*p) && result == p-><scalar>) { [p-><scalar> = 1;] return g(p, p); }   (the caller holds only reads: the minimum-amount rows)
    auto reads_only = [=](Kit& k, int shape, bool writes) -> const Stmt*
    {
        StructType* S {mkS(k, shape)};
        Function* g {make_g(k, S, shape)};
        Function* h {k.fn("h", k.i32, {{"p", k.ptr(k.st(S))}})};
        auto atom = [&]
        {
            return k.b.atom(AtomKind::Reads, k.b.text(k.F(h, 0), "p"), 0, k.at());
        };
        k.req(h, atom());
        k.ens(h, k.land(atom(), k.eq(k.R(h), k.leaf(k.b.text(k.F(h, 0), "p"), scalar_member(shape)))));
        std::vector<const Stmt*> body;
        const Stmt* w {nullptr};
        if (writes)
        {
            w = k.store(cells(k, shape, k.pv(h, 0))[scalar_cell(shape)], N(k, "1"));
            body.push_back(w);
        }
        body.push_back(k.ret(k.call(g, {k.pv(h, 0), k.pv(h, 0)})));
        h->body = k.blk(body);
        k.done(h);
        return w;
    };
    for (int shape : {0, 1, 2})
    {
        const std::string n {std::string("e1e2e_rrcall_") + names[shape]};
        lc(n + "_reads_only",
           std::string(docs[shape]) +
               ": h(p) holding only reads(*p) calls g(p, p): the two lent fractions are within what h holds, and h returns p's scalar",
           [=](Kit& k)
           {
               reads_only(k, shape, false);
           });
        lc(n + "_reads_only_writes_fail",
           std::string(docs[shape]) + ": the same writing p's scalar first: permission at the store",
           [=](Kit& k)
           {
               k.fail("permission", reads_only(k, shape, true)->loc);
           });
    }
    // two reads of ONE object where a callee has just lowered the caller's permission at a matrix cell (the sum form of audit round 3, B1, as rngc_reads_meet_kept for ranges): void keep_cell(int *x)
    // requires(reads(*x)) ensures(live(x)) {}; keep_cell(&u.m[1][1]); g(&u, &u): each atom's own amount would pass at the cell, the sum of the two does not: a permission error at the second call
    for (int shape : {0, 1})
    {
        const std::string n {std::string("e1e2e_rrcall_") + names[shape]};
        lc(n + "_meet_kept_fail",
           std::string(docs[shape]) +
               ": keep_cell(&u.m[1][1]) leaves the cell with a fraction, then g(&u, &u) takes two amounts of it: the sum of the two does not fit: permission at the call",
           [=](Kit& k)
           {
               StructType* S {mkS(k, shape)};
               Function* g {make_g(k, S, shape)};
               Function* keep {k.fn("keep_cell", k.vd, {{"x", k.ptr(k.i32)}})};
               k.req(keep, k.b.atom(AtomKind::Reads, k.b.text(k.F(keep, 0), "x"), 0, k.at()));
               k.ens(keep, k.b.atom(AtomKind::Live, k.b.text(k.F(keep, 0), "x"), 1, k.at()));
               keep->body = k.blk({});
               k.done(keep);
               Function* f {k.fn("f", k.vd)};
               Var* u {k.scell(f, "u", S)};
               std::vector<const Stmt*> body;
               fill(k, shape, u, body);
               body.push_back(k.es(k.call(keep, {cells(k, shape, k.addr(u))[shape == 0 ? 4U : 3U]})));
               const Expr* c2 {k.call(g, {k.addr(u), k.addr(u)})};
               body.push_back(k.es(c2));
               f->body = k.blkc(body);
               k.done(f);
               // the marked rows of the call's matrix part (a value row and a flag row for each of the two atoms) are four; the first, the sum-form row over every cell of every row, fails alone
               // with insufficient permission (a failed marked row ends its path)
               Expect e {"permission",          c2->loc.line, 4U, "assert.failed[insufficient.permission]",
                         "needs to read `u.m`", -1,           0};
               k.expects.push_back(e);
           });
    }
    // two GLOBAL structures read by two atoms: struct U gu, gv (n = 7, m = {{1, 2}, {3, 4}});  int f(void) requires(reads(gu) && reads(gv)) ensures(reads(gu) && reads(gv) && r == gu.n) { return gu.n; }
    // int main(void) { int rv = f(); check(rv == <7 | 8>); return 0; }
    auto global_pair = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* S {mkS(k, 0)};
        std::vector<Global*> gs;
        for (const char* name : {"gu", "gv"})
        {
            Global* g {k.prog.add_global(name, k.st(S), k.at(), nullptr)};
            GlobalLeaf ln;
            ln.kind = GlobalInit::Value;
            ln.init = N(k, "7");
            ln.value = "7";
            ln.loc = k.at();
            g->leaves.push_back(ln);
            GlobalLeaf lm;
            lm.kind = GlobalInit::Value;
            lm.rest = ArrayRest::Zero;
            lm.loc = k.at();
            for (const std::vector<const char*>& r : std::vector<std::vector<const char*>> {{"1", "2"}, {"3", "4"}})
            {
                GlobalLeaf row;
                row.kind = GlobalInit::Value;
                row.rest = ArrayRest::Zero;
                row.loc = k.at();
                for (const char* v : r)
                {
                    GlobalLeaf c;
                    c.kind = GlobalInit::Value;
                    c.value = v;
                    c.init = N(k, v);
                    c.loc = k.at();
                    row.elems.push_back(c);
                }
                lm.elems.push_back(row);
            }
            g->leaves.push_back(lm);
            gs.push_back(g);
        }
        auto atom = [&](int which)
        {
            return k.b.global_atom(AtomKind::Reads, k.b.global_addr(gs[static_cast<size_t>(which)], k.at()), which,
                                   k.at(), false);
        };
        auto gn = [&]
        {
            return k.deref(k.mem(k.b.global_addr(gs[0], k.at()), 0));
        };
        Function* f {k.fn("f", k.i32)};
        k.req(f, k.land(atom(0), atom(1)));
        k.ens(f, k.land(k.land(atom(0), atom(1)), k.eq(k.R(f), gn())));
        f->body = k.blk({k.ret(gn())});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        Var* rv {k.local(m, "rv", k.i32)};
        const Stmt* c {k.chk(k.eq(k.v(rv), N(k, wrong ? "8" : "7")))};
        m->body = k.blkc({k.decl(rv, k.call(f, {})), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c;
    };
    lc("e1e2e_rrcall_global_pair",
       "two global structures with a matrix member read by two atoms: f() takes reads(gu) && reads(gv); program_start gives gu.n == 7",
       [=](Kit& k)
       {
           global_pair(k, false);
       });
    lc("e1e2e_rrcall_global_pair_wrong_fail", "the same checking rv == 8: the check fails",
       [=](Kit& k)
       {
           k.fail("check", global_pair(k, true)->loc);
       });
}

// ---- the CASE SPLITS of a pointer the function obtained from elsewhere (split_candidates, decision 0037 item 3), beside a structure with a MATRIX member: the third internal error of the
// first-member-matrix family ("the leaves of an array element that is not a structure were asked for"): a member that is an array of ROWS went to the leaves of a structure element.  Shape 0
// struct U { int n; int m[2][2]; }, 1 struct U2 { int m[2][2]; int c; } ----
void define_part_e_split_matrix_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mkS = [](Kit& k, int shape) -> StructType*
    {
        const Type mat {k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)};
        return shape == 0 ? k.strukt("U", {{"n", k.i32}, {"m", mat}}) : k.strukt("U2", {{"m", mat}, {"c", k.i32}});
    };
    auto scalar = [](Kit& k, int shape, const Expr* base) -> const Expr*
    {
        return k.mem(base, shape == 0 ? 0 : 1);
    };
    auto mcell = [](Kit& k, int shape, const Expr* base, const char* i, const char* j) -> const Expr*
    {
        return k.b.elem_addr(k.b.elem_addr(k.mem(base, shape == 0 ? 1 : 0), k.num(k.i32, i), k.at()), k.num(k.i32, j),
                             k.at());
    };
    auto atom = [](Kit& k, Function* f, AtomKind kind, int param, int partner)
    {
        return k.b.atom(kind, k.b.text(k.F(f, param), f->params[static_cast<size_t>(param)].name), partner, k.at());
    };
    static const char* const names[] {"u", "u2"};
    static const char* const docs[] {"struct U { int n; int m[2][2]; }", "struct U2 { int m[2][2]; int c; }"};
    for (int shape : {0, 1})
    {
        const std::string n {std::string("e1e2e_split_") + names[shape]};
        const std::string d {docs[shape]};
        // int f(struct S *p, int *q) requires(reads(*p)) ensures(reads(*p)) { return q == NULL ? p-><scalar> : 0; }   (the auditor's q04: q has no atom; comparing it is a use of the pointer:
        // `lifetime` at the comparison, as for every pointer without an atom)
        lc(n + "_reads_param_unatomed_q_compared_fail",
           d + ": f(p, q) with reads(*p) and no atom on q, returning q == NULL ? p->scalar : 0: q is split against p's cells (the matrix's first cell among them); the comparison is a use of a pointer nothing holds: lifetime",
           [=](Kit& k)
           {
               StructType* S {mkS(k, shape)};
               Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.i32)}})};
               k.req(f, atom(k, f, AtomKind::Reads, 0, 0));
               k.ens(f, atom(k, f, AtomKind::Reads, 0, 0));
               const Expr* qv {k.pv(f, 1)};
               const Expr* cmp {k.eq(qv, k.nul(k.ptr(k.i32)))};
               f->body = k.blk({k.ret(k.cond(cmp, k.deref(scalar(k, shape, k.pv(f, 0))), N(k, "0")))});
               k.done(f);
               k.fail("lifetime", qv->loc);
           });
        // void f(struct S *p, int *q) requires(owns(*p)) ensures(owns(*p)) { *q = 1; }   (q has no atom: the store is held by nothing)
        lc(n + "_owns_param_unatomed_q_write_fail",
           d + ": f(p, q) with owns(*p) and no atom on q writing *q: permission at the store (q may point anywhere; the split against p's cells does not hold it)",
           [=](Kit& k)
           {
               StructType* S {mkS(k, shape)};
               Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.i32)}})};
               k.req(f, atom(k, f, AtomKind::Owns, 0, 0));
               k.ens(f, atom(k, f, AtomKind::Owns, 0, 0));
               const Stmt* w {k.store(k.pv(f, 1), N(k, "1"))};
               f->body = k.blk({w});
               k.done(f);
               k.fail("null-dereference", w->loc);
               k.fail("permission", w->loc);
           });
        // int *pick(int *a, int *b, int c) requires(owns(*a) && owns(*b)) ensures(owns(*a) && owns(*b) && r == (c != 0 ? a : b)) trusted;
        // void f(int c) { struct S u, w; <cells written>; int *r = pick(<&u.x>, <&w.x>, c); *r = 5; check(<u.x == 5 || w.x == 5 | u.x == 5>); }
        // `cellkind`: 0 the scalar, 1 the matrix cell [1][1]
        auto local_pick = [=](Kit& k, int cellkind, bool wrong) -> const Stmt*
        {
            StructType* S {mkS(k, shape)};
            Function* g {k.fn("pick", k.ptr(k.i32), {{"a", k.ptr(k.i32)}, {"b", k.ptr(k.i32)}, {"c", k.i32}})};
            k.req(g, k.land(atom(k, g, AtomKind::Owns, 0, 0), atom(k, g, AtomKind::Owns, 1, 1)));
            k.ens(g, k.land(k.land(atom(k, g, AtomKind::Owns, 0, 0), atom(k, g, AtomKind::Owns, 1, 1)),
                            k.eq(k.R(g), k.cond(k.ne(k.F(g, 2), N(k, "0")), k.F(g, 0), k.F(g, 1)))));
            k.trusted(g);
            Function* f {k.fn("f", k.vd, {{"c", k.i32}})};
            Var* u {k.scell(f, "u", S)};
            Var* w {k.scell(f, "w", S)};
            Var* r {k.local(f, "r", k.ptr(k.i32))};
            auto cell = [&](const Var* x) -> const Expr*
            {
                return cellkind == 0 ? scalar(k, shape, k.addr(x)) : mcell(k, shape, k.addr(x), "1", "1");
            };
            std::vector<const Stmt*> body {k.decl(u, nullptr), k.decl(w, nullptr)};
            for (const Var* x : {u, w})
            {
                body.push_back(k.store(cell(x), N(k, "1")));
            }
            body.push_back(k.decl(r, k.call(g, {cell(u), cell(w), k.pv(f, 0)})));
            body.push_back(k.store(k.v(r), N(k, "5")));
            const Expr* is_u {k.eq(k.deref(cell(u)), N(k, "5"))};
            const Stmt* c {k.chk(wrong ? is_u : k.lor(is_u, k.eq(k.deref(cell(w)), N(k, "5"))))};
            body.push_back(c);
            f->body = k.blkc(body);
            k.done(f);
            k.text_checks.push_back(
                [=](const std::string& vpr) -> std::string
                {
                    // the result is split against the first cells of each of the two structures: `if (t__n == <cell>) { }` for the scalar's cell `loc(member(v_u__m, s), 0)` and the matrix's
                    // first cell `loc(member(loc(member(v_u__m, k), 0), 0), 0)`: four distinct splits
                    static const std::regex split {
                        R"(if \(t__[0-9]+ == (loc\(member\((?:loc\(member\()?v_[a-z]+__[0-9]+, [01]\)(?:, 0\), 0\))?, 0\))\) \{ \})"};
                    std::set<std::string> seen;
                    for (std::sregex_iterator it {vpr.begin(), vpr.end(), split}, end; it != end; ++it)
                    {
                        seen.insert((*it)[1].str());
                    }
                    return seen.size() == 4U
                               ? ""
                               : "expected four case splits of the result (the scalar's cell and the matrix's first cell of each structure), found " +
                                     std::to_string(seen.size());
                });
            return c;
        };
        lc(n + "_local_pick_scalar",
           d + ": r = pick(&u.x, &w.x, c) beside two locals of the type; *r = 5; one of them holds 5 (the result is split against the cells of both structures, the matrix's first cell among them)",
           [=](Kit& k)
           {
               local_pick(k, 0, false);
           });
        lc(n + "_local_pick_scalar_wrong_fail",
           d + ": the same checking u.x == 5 alone (r may be w's cell): the check fails",
           [=](Kit& k)
           {
               k.fail("check", local_pick(k, 0, true)->loc);
           });
        lc(n + "_local_pick_matrix_cell",
           d + ": r = pick(&u.m[1][1], &w.m[1][1], c); *r = 5: the arguments are matrix cells (the split names only each matrix's first cell: a no-op branch, the write still verifies)",
           [=](Kit& k)
           {
               local_pick(k, 1, false);
           });
        lc(n + "_local_pick_matrix_cell_wrong_fail", d + ": the same checking u.m[1][1] == 5 alone: the check fails",
           [=](Kit& k)
           {
               k.fail("check", local_pick(k, 1, true)->loc);
           });
        // void g(struct S *p, int *q) requires(owns(*p)) trusted;  void f(void) { struct S u; <cells>; g(&u, &u.m[1][1]); }   (the pointer IS a matrix cell's address; q has no atom)
        lc(n + "_cell_address_to_unatomed_q",
           d + ": g(&u, &u.m[1][1]) with owns(*p) and no atom on q: accepted (no obligation names q)",
           [=](Kit& k)
           {
               StructType* S {mkS(k, shape)};
               Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(S))}, {"q", k.ptr(k.i32)}})};
               k.req(g, atom(k, g, AtomKind::Owns, 0, 0));
               k.ens(g, atom(k, g, AtomKind::Owns, 0, 0));
               k.trusted(g);
               Function* f {k.fn("f", k.vd)};
               Var* u {k.scell(f, "u", S)};
               std::vector<const Stmt*> body {k.decl(u, nullptr)};
               for (const char* i : {"0", "1"})
               {
                   for (const char* j : {"0", "1"})
                   {
                       body.push_back(k.store(mcell(k, shape, k.addr(u), i, j), N(k, "1")));
                   }
               }
               body.push_back(k.store(scalar(k, shape, k.addr(u)), N(k, "1")));
               body.push_back(k.es(k.call(g, {k.addr(u), mcell(k, shape, k.addr(u), "1", "1")})));
               f->body = k.blkc(body);
               k.done(f);
           });
        // int f(struct S *p, int **pp) requires(owns(*p) && owns(*pp)) ensures(owns(*p) && owns(*pp)) { int *r = *pp; return r == NULL ? p-><scalar> : 0; }   (a pointer loaded from memory)
        lc(n + "_loaded_pointer_compared_fail",
           d + ": a pointer loaded from memory beside owns(*p): split against p's cells, compared with NULL: lifetime at the comparison",
           [=](Kit& k)
           {
               StructType* S {mkS(k, shape)};
               Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(S))}, {"pp", k.ptr(k.ptr(k.i32))}})};
               k.req(f, k.land(atom(k, f, AtomKind::Owns, 0, 0), atom(k, f, AtomKind::Owns, 1, 1)));
               k.ens(f, k.land(atom(k, f, AtomKind::Owns, 0, 0), atom(k, f, AtomKind::Owns, 1, 1)));
               Var* r {k.local(f, "r", k.ptr(k.i32))};
               const Expr* load {k.deref(k.pv(f, 1))};
               const Expr* cmp {k.eq(k.v(r), k.nul(k.ptr(k.i32)))};
               f->body =
                   k.blkc({k.decl(r, load), k.ret(k.cond(cmp, k.deref(scalar(k, shape, k.pv(f, 0))), N(k, "0")))});
               k.done(f);
               // the loaded pointer is a use of a pointer nothing holds: the lifetime row sits at the load
               k.fail("lifetime", load->loc);
           });
    }
}
