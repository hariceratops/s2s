// Ported from test/constexpr/field_list_metadata.cpp. The 47 static_asserts
// there are carried over unchanged in meaning; the file's `main`, which
// printed two of the same results to cout, is gone.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently. The predicates below are
// namespace scope because they are NTTPs of the schema.

#include <cstddef>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;

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

using legal_len_field_list = 
  s2s::field_list_metadata<
    s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

using illegal_len_field_list = 
  s2s::field_list_metadata<
    s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", int, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>
  >;

int main() {
  "the field table places every field at its declaration index"_test = [] constexpr {
    constexpr auto table = list_metadata::field_table;

    expect(eq(static_cast<bool>(table["unknown"]), false));
    expect(eq(table["a"]->occurs_at_idx, std::size_t{0}));
    expect(eq(table["b"]->occurs_at_idx, std::size_t{1}));
    expect(eq(table["len"]->occurs_at_idx, std::size_t{2}));
    expect(eq(table["str"]->occurs_at_idx, std::size_t{3}));
    expect(eq(table["row"]->occurs_at_idx, std::size_t{4}));
    expect(eq(table["col"]->occurs_at_idx, std::size_t{5}));
    expect(eq(table["flat_vec"]->occurs_at_idx, std::size_t{6}));
    expect(eq(table["c"]->occurs_at_idx, std::size_t{7}));
    expect(eq(table["vec"]->occurs_at_idx, std::size_t{8}));
    expect(eq(table["d"]->occurs_at_idx, std::size_t{9}));
    expect(eq(table["v"]->occurs_at_idx, std::size_t{10}));
    expect(eq(table["vec_union"]->occurs_at_idx, std::size_t{11}));
  };

  "the length dependency table names each field's size sources"_test = [] constexpr {
    constexpr auto table = list_metadata::length_dependency_table;

    expect(eq(table["a"]->size(), std::size_t{0}));
    expect(eq(table["b"]->size(), std::size_t{0}));
    expect(eq(table["len"]->size(), std::size_t{0}));
    expect(eq(table["row"]->size(), std::size_t{0}));
    expect(eq(table["col"]->size(), std::size_t{0}));
    expect(eq(table["c"]->size(), std::size_t{0}));
    expect(eq(table["d"]->size(), std::size_t{0}));
    expect(eq(table["v"]->size(), std::size_t{0}));

    expect(eq(table["str"]->size(), std::size_t{1}));
    constexpr auto str_dep = *table["str"];
    expect(eq(str_dep[0], std::string_view{"len"}));

    // Two sources, in the order the callable takes them.
    constexpr auto vec_dep = *table["flat_vec"];
    expect(eq(vec_dep[0], std::string_view{"row"}));
    expect(eq(vec_dep[1], std::string_view{"col"}));

    expect(eq(table["vec"]->size(), std::size_t{1}));
    expect(eq(table["vec_union"]->size(), std::size_t{1}));
  };

  "the parse dependency table names the predicate's sources"_test = [] constexpr {
    constexpr auto table = list_metadata::parse_dependency_table;

    expect(eq(table["vec"]->size(), std::size_t{1}));
    expect(eq(table["a"]->size(), std::size_t{0}));
  };

  "the type deduction table names each union's sources"_test = [] constexpr {
    constexpr auto table = list_metadata::type_deduction_dep_table;

    expect(eq(static_cast<bool>(table["v"]), true));
    expect(eq(table["v"]->size(), std::size_t{1}));
    expect(eq(static_cast<bool>(table["complex_v"]), true));
    expect(eq(table["complex_v"]->size(), std::size_t{2}));
    expect(eq(static_cast<bool>(table["laddered"]), true));
    expect(eq(table["laddered"]->size(), std::size_t{2}));
  };

  // Derived = unconditional len_from_field targets, plus match_field switch
  // discriminants. "len" qualifies through the top-level str_field and stays
  // derived even though it also has conditional producers inside a maybe and a
  // union alternative. "a" qualifies as the discriminant of "v" and
  // "vec_union": derived from the union, while the optional's length is
  // verified against that derived value.
  "only invertible obligations make a field derived"_test = [] constexpr {
    constexpr auto ids = list_metadata::derived_field_ids;
    expect(eq(ids.size(), std::size_t{2}));

    expect(eq(s2s::is_derived_field<meta::type_id<list_metadata>>("len"), true));
    expect(eq(s2s::is_derived_field<meta::type_id<list_metadata>>("a"), true));
    // "row" and "col" feed a len_from_fields callable, which has no inverse.
    expect(eq(s2s::is_derived_field<meta::type_id<list_metadata>>("row"), false));
    expect(eq(s2s::is_derived_field<meta::type_id<list_metadata>>("col"), false));
    expect(eq(s2s::is_derived_field<meta::type_id<list_metadata>>("str"), false));
    // "complex_v" switches on a computed value and "laddered" on branch
    // predicates; neither can be inverted, so neither obligates anything.
    expect(eq(s2s::is_derived_field<meta::type_id<list_metadata>>("b"), false));
  };

  "a length declared before its dependent resolves"_test = [] constexpr {
    expect(eq(s2s::size_dependencies_resolved<legal_len_field_list>(), true));
  };

  // Same two fields, but "len" is declared after the "str" that needs it, so
  // the value is not available when the read reaches "str".
  "a length declared after its dependent does not resolve"_test = [] constexpr {
    constexpr auto table = illegal_len_field_list::field_table;
    expect(eq(table["str"]->occurs_at_idx, std::size_t{2}));
    expect(eq(table["len"]->occurs_at_idx, std::size_t{4}));

    constexpr auto deps = *illegal_len_field_list::length_dependency_table["str"];
    expect(eq(deps[0], std::string_view{"len"}));

    expect(eq(s2s::size_dependencies_resolved<illegal_len_field_list>(), false));
  };
}
