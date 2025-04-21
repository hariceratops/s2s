#include "../single_header/struct_cast.hpp"

namespace static_tests {

using u32 = unsigned int;
namespace tl = typelist;

static_assert(is_no_constraint_v<no_constraint<int>>);
static_assert(!is_no_constraint_v<int>);
static_assert(is_no_constraint_v<no_constraint<std::optional<int>>>);

static_assert(!are_unique_types_v<field_choice_list<int, int, float>>);
static_assert(are_unique_types_v<field_choice_list<int, double, float>>);
static_assert(are_unique_types_v<field_choice_list<int, std::vector<double>, std::vector<float>>>);

static_assert(!has_unique_field_ids_v<"hello", "world", "hello">);
static_assert(has_unique_field_ids_v<"hello", "world", "nexus">);

static_assert(is_variable_size_v<field_size<len_from_field<"hello">>>);
static_assert(is_fixed_size_v<field_size<fixed<4>>>);
static_assert(!is_fixed_size_v<int>);
static_assert(!is_variable_size_v<int>);
static_assert(field_size<fixed<6>>::size_type_t::count == 6);

static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);

static inline auto is_eq_1 = [](auto a){ return a == 1; };
static_assert(is_optional_field_v<maybe_field<"a", u32, field_size<fixed<4>>, parse_if<is_eq_1, with_fields<"a">>>>);
static_assert(!is_optional_field_v<basic_field<"a", u32, field_size<fixed<4>>>>);

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

static_assert(fixed_string("hello").size() == 5);


static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);


static_assert(std_read_trait<std::ifstream>);
static_assert(readable<std::ifstream>);
static_assert(convertible_to_bool<std::ifstream>);

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

static_assert(std::is_same_v<variant_from_tlist_v<typelist::typelist<int, float>>, std::variant<int, float>>);

static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
static_assert(
std::is_same_v<extract_type_from_field_v<
  struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
  struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
);

// todo static tests
//// namespace static_test {
//   // implicit conversion produces false positive in second static_assert, 
//   // must be checked with std::is_same_v additionally
//   // auto constexpr unit = [](int a)-> int { return a * 1; };
//   // static_assert(std::is_invocable_r_v<int, decltype(unit), int>);
//   // static_assert(std::is_invocable_r_v<float, decltype(unit), int>);
//   // static_assert(std::is_invocable_r_v<char*, decltype(unit), int>);
//
//   auto unit = [](auto a){ return a * 1; };
//   auto is_a_eq_1 = [](auto a){ return a == 1; };
//   using test_s2s::struct_field_list = 
//     s2s::struct_field_list<
//       s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
//       s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
//     >;
//   static_assert(is_invocable<is_a_eq_1, bool, test_s2s::struct_field_list, s2s::with_fields<"a">>::res);
//   static_assert(can_eval_R_from_fields<is_a_eq_1, int, test_s2s::struct_field_list, s2s::with_fields<"a">>);
//   // static_assert(can_eval_R_from_fields<is_a_eq_1, char*, test_s2s::struct_field_list, s2s::with_fields<"a">>);
//   static_assert(is_invocable<unit, bool, test_s2s::struct_field_list, s2s::with_fields<"a">>::res);
// }
  // using inner =  
  //   struct_field<
  //       "c", 
  //       s2s::struct_field_list<
  //         s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
  //         s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  //       >
  //     >;
  // static_assert(struct_field_like<inner>);
  // static_assert(variable_sized_field_like<inner>);
  // static_assert(s2s::fixed_sized_field_like<inner>);
// using temp = 
//   s2s::struct_field_list<
//     s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
//     s2s::basic_field<"b", int, s2s::field_size<s2s::fixed<4>>>
//   >;
// will fail
// using non_unique_temp = 
//   s2s::struct_field_list<
//     s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>,
//     s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<4>>>
//   >;
// using u32 = unsigned int;
// static_assert(array_of_records_like<std::array<temp, 10>>);
// static_assert(vector_of_records_like<std::vector<temp>>);
// static_assert(!field_containable<std::array<temp, 10>>);
// will fail since type choices are not unique
// using test_s2s::struct_field_list = 
//     s2s::struct_field_list<
//       s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
//       s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
//       union_field<
//         "c", 
//         type<
//           match_field<"a">,
//           type_switch<
//             match_case<0xcafed00d, type_tag<float, s2s::field_size<s2s::fixed<4>>>>,
//             match_case<0xdeadbeef, type_tag<int, s2s::field_size<s2s::fixed<4>>>>,
//             match_case<0xbeefbeef, type_tag<int, s2s::field_size<s2s::fixed<4>>>>
//           >
//         >
//       >
//     >;
//
// will fail!
// using size_failure_s2s::basic_field = s2s::basic_field<"a", int, s2s::field_size<s2s::fixed<6>>>;


}
