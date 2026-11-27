// (included by selftest.cpp) Arrays, M4.5e1a Parts A and B (decision 0059 items 2 and 4): the type `Array` and the node `ElemAddr` exist in VIR, and check_program
// accepts only an automatic array local (a cell of a scalar or accepted-pointer element) and `ElemAddr` over its address; every other position that could hold
// one is a check error naming it.  These are the rejection cases of that fail-closed state; the lowering cases of the accepted shapes are in the later files.
namespace {

void define_arrays_rejected_until_part_b()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto A3 = [](Kit& k)
    {
        return k.prog.array_of(k.i32, 3U);
    };
    // ---- the node `ElemAddr`, in every position the accepted shapes do not cover (the pointer-based and array-based element places are lowering cases) ----
    rc("arr_rej_elem_addr_over_struct_pointer", "or an `owning` or `ref_to_uninit` member",
       [=](Kit& k)
       {
           // sp[1] of a `struct S *sp` whose structure holds an array member (decision 0064 Part B: the element of a pointer to a structure is accepted, one with an array member is Part E's;
           // flipped from "structures: M4.5e1e")
           // (slice 3c-2 accepts an array member of scalars; a matrix member, depth 3, stays refused)
           StructType* S {k.strukt("S", {{"x", k.i32}, {"a", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)}})};
           Function* f {k.fn("f", k.vd, {{"sp", k.ptr(k.st(S))}})};
           f->body = k.blk({k.es(k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at()))});
           k.done(f);
       });
    rc("arr_rej_elem_addr_in_atom", "the address of an array element in a contract",
       [=](Kit& k)
       {
           // requires(owns(p[1]))
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.b.elem_addr(k.F(f, 0), N(k, "1"), k.at()), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_elem_addr_ill_typed_base", "of anything but a local array or a pointer",
       [=](Kit& k)
       {
           // a base that is no pointer is built as it is and refused
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", k.i32)};
           f->body = k.blk({k.decl(q, N(k, "0")), k.es(k.deref(k.b.elem_addr(N(k, "0"), N(k, "1"), k.at())))});
           k.done(f);
       });

    // ---- the type `Array`, in every position ----
    rc("arr_rej_array_local_not_a_cell", "internal: the array local",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.local(f, "a", A3(k))};
           f->body = k.blk({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_parameter", "is not a bool, an integer type or a pointer to one",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"a", A3(k)}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_array_result", "is not void, bool, an integer type or a pointer to one",
       [=](Kit& k)
       {
           Function* f {k.fn("f", A3(k))};
           k.trusted(f);
           (void)f;
       });
    rc("arr_rej_pointer_to_array_local", "a pointer to int[3]",
       [=](Kit& k)
       {
           // int (*q)[3];
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", k.ptr(A3(k)))};
           f->body = k.blk({k.decl(q, nullptr)});
           k.done(f);
       });
    // (Part E, slice 2 lifts `int (*q)[3]` as a parameter, record 0064 item 13; the case now holds the parameter that stays refused until slice 4: a pointer to an array of arrays, `int (*q)[2][3]`)
    rc("arr_rej_pointer_to_array_parameter", "a pointer to int[3]",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"q", k.ptr(k.prog.array_of(A3(k), 2U))}})};
           f->body = k.blk({});
           k.done(f);
       });
    rc("arr_rej_array_value", "an array has no value",
       [=](Kit& k)
       {
           // a read of a whole array through a pointer parameter: `*q` of an `int (*q)[3]`
           Function* f {k.fn("f", k.vd, {{"q", k.ptr(A3(k))}})};
           f->body = k.blk({k.es(k.deref(k.pv(f, 0)))});
           k.done(f);
       });
    rc("arr_rej_array_struct_member_zero_length", "of no elements",
       [=](Kit& k)
       {
           // struct S { int a[0]; }; (an array member is accepted from M4.5e1e1 part C: one dimension of at least one element)
           k.strukt("S", {{"a", k.prog.array_of(k.i32, 0U)}});
       });
    rc("arr_rej_array_global_of_structs_missing_leaves", "has 0 initial values for the 1 leaves",
       [=](Kit& k)
       {
           // struct T { int x; }; struct T g[3] = {...}; with a listed element that has no value for its leaf (the global array of structures is accepted from M4.5e1e2 part C2)
           StructType* T {k.strukt("T", {{"x", k.i32}})};
           Global* g {k.prog.add_global("g", k.prog.array_of(k.st(T), 3U), k.at(), nullptr)};
           g->elems.push_back(GlobalLeaf {});
       });
    rc("arr_rej_array_zero_length", "of no elements",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(k.i32, 0U))};
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_of_void_pointers", "is an array of",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(k.ptr(k.vd), 2U))};
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_with_an_initializer_value", "declared with an initializer value",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           f->body = k.blkc({k.decl(a, N(k, "0"))});
           k.done(f);
       });
    rc("arr_rej_array_owning", "is marked owning",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(PI(k), 2U))};
           a->owning = true;
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
    rc("arr_rej_zero_filled_scalar", "is marked zero-filled but is not an array",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* x {k.local(f, "x", k.i32)};
           x->zero_filled = true;
           f->body = k.blk({k.decl(x, nullptr)});
           k.done(f);
       });
    rc("arr_rej_array_address_as_a_value", "outside an element address",
       [=](Kit& k)
       {
           // int *q = a; (the decay, Part D)
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           Var* q {k.local(f, "q", PI(k))};
           f->body = k.blkc({k.decl(a, nullptr), k.decl(q, k.addr(a))});
           k.done(f);
       });
    rc("arr_rej_element_index_not_an_integer", "not an integer",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           f->body = k.blkc({k.decl(a, nullptr), k.es(k.deref(k.b.elem_addr(k.addr(a), k.yes(), k.at())))});
           k.done(f);
       });
    rc("arr_rej_element_index_enum", "not an integer",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", A3(k))};
           const Type en {k.en("E", 32, true, "0", "2")};
           f->body = k.blkc({k.decl(a, nullptr), k.es(k.deref(k.b.elem_addr(k.addr(a), k.num(en, "1"), k.at())))});
           k.done(f);
       });
    rc("arr_rej_array_of_arrays_local", "is an array",
       [=](Kit& k)
       {
           // int a[2][3][4]: built as is, refused.  (M4.5e1e2 Part E slice 1 lifts `int a[2][3]` for an automatic local: that matrix is the case e1e2e_matrix_*; a depth-3 array of arrays is slice 4's.)
           Function* f {k.fn("f", k.vd)};
           Var* a {k.cell(f, "a", k.prog.array_of(k.prog.array_of(A3(k), 2U), 4U))};
           f->body = k.blkc({k.decl(a, nullptr)});
           k.done(f);
       });
}

} // namespace
