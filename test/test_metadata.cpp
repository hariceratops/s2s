#include "../single_header/s2s.hpp"

using u32 = unsigned int;


auto size_from_rc = [](auto r, auto c) { return r * c; };
auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
auto some_complex_calc = [](auto a, auto b){ return a + b; };
auto bpred_1 = [](auto a, auto b){ return a + b >= 20000 && a + b < 40000; };
auto bpred_2 = [](auto a, auto b){ return a + b <= 40000 && a + b < 60000; };
auto bpred_3 = [](auto a, auto b){ return a + b >= 60000; };

using inner_1 = 
 s2s::struct_field_list<
   s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>, 
   s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
>;
using inner_2 = 
 s2s::struct_field_list<
   s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
   s2s::basic_field<"q", u32, s2s::field_size<s2s::fixed<4>>>
>;

using list_metadata =
  s2s::field_list_metadata<
    s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::basic_field<"row", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::basic_field<"col", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::vec_field<
      "flat_vec",
      u32,
      s2s::field_size<
        s2s::len_from_fields<size_from_rc, s2s::with_fields<"row", "col">>
      >
    >,
    s2s::maybe<
      s2s::basic_field<"c", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
    >,
    s2s::maybe<
      s2s::vec_field<
        "vec", 
        u32, 
        s2s::field_size<s2s::len_from_field<"len">>
      >, 
      s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
    >,
    s2s::struct_field<
      "d",
      s2s::struct_field_list<
         s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
         s2s::basic_field<"q", u32, s2s::field_size<s2s::fixed<4>>>
      >
    >,
    s2s::variance<
      "v", 
      s2s::type<
        s2s::match_field<"a">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<inner_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<inner_2>>
        >
      >
    >,
    s2s::variance<
      "vec_union", 
      s2s::type<
        s2s::match_field<"a">,
        s2s::type_switch<
          s2s::match_case<
            0xcafed00d, 
            s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::match_case<
            0xdeadbeef, 
            s2s::as_vec<u32, s2s::field_size<s2s::len_from_field<"len">>>
          >,
          s2s::match_case<
            0xbeefbeef, 
            s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >,
    s2s::variance<
      "complex_v", 
      s2s::type<
        s2s::compute<some_complex_calc, u32, s2s::with_fields<"a", "b">>,
        s2s::type_switch<
          s2s::match_case<
            100, 
            s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::match_case<
            200, 
            s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::match_case<
            300, 
            s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >,
    s2s::variance<
      "laddered", 
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<bpred_1, s2s::with_fields<"a", "b">>, 
            s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::branch<
            s2s::predicate<bpred_2, s2s::with_fields<"a", "b">>, 
            s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::branch<
            s2s::predicate<bpred_3, s2s::with_fields<"a", "b">>, 
            s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >
  >;

constexpr auto fields_table = list_metadata::field_table;
static_assert(not fields_table["unknown"]);
static_assert(fields_table["a"]->occurs_at_idx == 0);
static_assert(fields_table["b"]->occurs_at_idx == 1);
static_assert(fields_table["len"]->occurs_at_idx == 2);
static_assert(fields_table["str"]->occurs_at_idx == 3);
static_assert(fields_table["row"]->occurs_at_idx == 4);
static_assert(fields_table["col"]->occurs_at_idx == 5);
static_assert(fields_table["flat_vec"]->occurs_at_idx == 6);
static_assert(fields_table["c"]->occurs_at_idx == 7);
static_assert(fields_table["vec"]->occurs_at_idx == 8);
static_assert(fields_table["d"]->occurs_at_idx == 9);
static_assert(fields_table["v"]->occurs_at_idx == 10);
static_assert(fields_table["vec_union"]->occurs_at_idx == 11);

constexpr auto len_dep_table = list_metadata::length_dependency_table;
static_assert(len_dep_table["a"]->size() == 0);
static_assert(len_dep_table["b"]->size() == 0);
static_assert(len_dep_table["len"]->size() == 0);
static_assert(len_dep_table["str"]->size() == 1);
constexpr auto str_len_dep = *len_dep_table["str"];
static_assert(str_len_dep[0] == "len");
static_assert(len_dep_table["row"]->size() == 0);
static_assert(len_dep_table["col"]->size() == 0);
constexpr auto vec_len_dep = *len_dep_table["flat_vec"];
static_assert(vec_len_dep[0] == "row");
static_assert(vec_len_dep[1] == "col");
static_assert(len_dep_table["c"]->size() == 0);
static_assert(len_dep_table["d"]->size() == 0);
static_assert(len_dep_table["vec"]->size() == 1);
static_assert(len_dep_table["v"]->size() == 0);
static_assert(len_dep_table["vec_union"]->size() == 1);


constexpr auto parse_dep_table = list_metadata::parse_dependency_table;
static_assert(parse_dep_table["vec"]->size() == 1);
static_assert(parse_dep_table["a"]->size() == 0);

constexpr auto type_deduction_dep_table = list_metadata::type_deduction_dep_table;
static_assert(type_deduction_dep_table["v"]);
static_assert(type_deduction_dep_table["v"]->size() == 1);
static_assert(type_deduction_dep_table["complex_v"]);
static_assert(type_deduction_dep_table["complex_v"]->size() == 2);
static_assert(type_deduction_dep_table["laddered"]);
static_assert(type_deduction_dep_table["laddered"]->size() == 2);

using legal_len_field_list = 
  s2s::field_list_metadata<
    s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

constexpr bool res_legal = s2s::size_dependencies_resolved<legal_len_field_list>();
static_assert(res_legal);

using illegal_len_field_list = 
  s2s::field_list_metadata<
    s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>
  >;

constexpr auto illegal_fields_table = illegal_len_field_list::field_table;
static_assert(illegal_fields_table["str"]->occurs_at_idx == 2);
static_assert(illegal_fields_table["len"]->occurs_at_idx == 4);
constexpr auto illegal_len_dep_table = illegal_len_field_list::length_dependency_table;
constexpr auto illegal_str_len_dep = *illegal_len_dep_table["str"];
static_assert(illegal_str_len_dep[0] == "len");

constexpr bool res_illegal = s2s::size_dependencies_resolved<illegal_len_field_list>();
static_assert(not res_illegal);


auto main(void) -> int {
  bool res_legal = s2s::size_dependencies_resolved<legal_len_field_list>();
  bool res_illegal = s2s::size_dependencies_resolved<illegal_len_field_list>();
  std::cout << res_legal << " " << res_illegal << '\n';

  auto bpred_1 = [](auto a, auto b){ return a + b >= 20000 && a + b < 40000; };
  auto bpred_2 = [](auto a, auto b){ return a + b <= 40000 && a + b < 60000; };
  auto bpred_3 = [](auto a, auto b){ return a + b >= 60000; };
  using union_field = 
      s2s::variance<
        "c", 
        s2s::type<
          s2s::type_if_else<
            s2s::branch<
              s2s::predicate<bpred_1, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::branch<
              s2s::predicate<bpred_2, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::branch<
              s2s::predicate<bpred_3, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >;

  auto constexpr vec = meta::invoke<s2s::extract_type_deduction_dependencies>(meta::type_id<union_field>);
  std::cout << vec[0] << " " << vec[1] << '\n';
  return 0;
}

