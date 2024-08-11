#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include "../typelist.hpp"
#include "../struct_field_list_base.hpp"
#include <functional>
#include <optional>
#include "../field_accessor.hpp"
#include "field_reader.hpp"
#include "field_list.hpp"


template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;


struct empty_list{};
struct not_a_list{};
struct out_of_bound{};


template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
    static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
    static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
    static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
    static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
    static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
    using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
    using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
    using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

template <typename T>
concept field_name_list = is_field_name_list_v<T>;

// todo add constriants
template <auto callable, typename R, field_name_list fstr_list>
struct compute;


// template <typename Func, typename... Ts>
// concept eval_bool_from_fields = 
//   requires {
//     { Func(Ts{}...) } -> std::same_as<bool>;
//   };
//
// template <typename Func, typename... Ts>
// concept eval_size_from_fields = 
//   requires {
//     { Func(Ts{}...) } -> std::same_as<std::size_t>;
//   };
//

template <auto callable, typename R, typename struct_fields, typename field_list, typename indices>
struct is_invocable;

template <auto callable, 
          typename R, 
          typename struct_fields, 
          fixed_string_like req_field_list, 
          std::size_t... idx>
struct is_invocable<callable, 
                    R, 
                    struct_fields, 
                    req_field_list, 
                    std::index_sequence<idx...>> {
  static constexpr bool res = 
      std::is_invocable_r<
        R, 
        decltype(callable),
        decltype(struct_fields{}[field_accessor<front_t<pop_t<idx, req_field_list>>>{}])...
      >::value; 
};

template <auto Func, 
          typename R, 
          typename struct_fields, 
          typename req_fields>
concept can_eval_R_from_fields = 
  is_invocable<
    Func, 
    R, 
    struct_fields, 
    req_fields,
    std::make_integer_sequence<std::size_t, size_v<req_fields>>
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?
template <auto callable, typename R, field_name_list req_fields>
struct compute {
  template <typename... fields, std::size_t... idx>
  constexpr auto invoke_impl(struct_field_list<fields...>& flist, std::index_sequence<idx...>) {
    return std::invoke(callable, flist[field_accessor<front_t<pop_t<idx, req_fields>>>{}]...);
  }

  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                req_fields>)
  constexpr auto operator()(struct_field_list<fields...>& flist) {
    return invoke_impl(flist, std::make_integer_sequence<std::size_t, size_v<req_fields>>{});
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;


template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute<callable, R, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_compute_like_v = is_compute_like<T>::res;

template <typename T>
struct is_eval_bool_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_bool_from_fields<compute<callable, bool, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_bool_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_bool_from_fields_v = is_eval_bool_from_fields<T>::res;

template <typename T>
struct is_eval_size_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_size_from_fields<compute<callable, std::size_t, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;

// todo move to another[separate] file
// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;


// todo is this required
template <typename T, typename size>
struct type_tag {
  using type = T;
  using field_size = size;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<type_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_type_tag_v = is_type_tag<T>::res;

// todo constrain to data types possible for fields
// todo constrain T?
template <auto v, typename T>
struct match_case {
  static constexpr auto value = v;
  using type_tag = T;
};


template <typename T>
struct is_match_case;

template <auto v, typename h>
struct is_match_case<match_case<v, h>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_match_case {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_match_case_v = is_match_case<T>::res;

template <typename T>
concept match_case_like = is_match_case_v<T>;


template <std::size_t idx, typename... cases>
struct type_switch_impl;

template <std::size_t idx>
struct type_switch_impl<idx> {
  constexpr auto operator()(auto) const -> 
    std::expected<std::size_t, std::string> 
  {
    return std::unexpected("no matches found");
  }
};

// todo check if case and eval result match in terms of types
template <std::size_t idx, match_case_like match_case_head, match_case_like... match_case_rest>
struct type_switch_impl<idx, match_case_head, match_case_rest...> {
  constexpr auto operator()(auto v) const -> 
    std::expected<std::size_t, std::string> 
  {
    if(v == match_case_head::value) return idx;
    else return type_switch_impl<idx + 1, match_case_rest...>{}(v);
  }
};

// atleast one type has to match? but anyways if nothing is matches we get 
// std::unexpected
// todo constrain eval to compute type, cases to match cases
// todo constrain eval return type matches all match case values
// todo return tag constructed with match
template <typename eval, match_case_like case_head, match_case_like... case_rest>
  requires is_compute_like_v<eval>
struct type_switch {
  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    auto expression = eval{};
    auto v = expression(field_list);
    return type_switch_impl<0, case_head, case_rest...>{}(v);
  } 
};

// todo constrain v to function like object returning bool
template <typename eval, typename T>
  requires is_eval_bool_from_fields_v<eval>
struct clause {
  static constexpr auto e = eval{};
  using type_tag = T;
};

// todo return type tag constructed from clause
template <typename... clauses>
struct type_ladder;

template <std::size_t idx, typename... clauses>
struct type_ladder_impl;

template <std::size_t idx>
struct type_ladder_impl<idx> {
  constexpr auto operator()(auto) const -> 
    std::expected<std::size_t, std::string> 
  {
    return std::unexpected("no matches found");
  }
};

// todo constrain clause head and clause_rest
template <std::size_t idx, typename clause_head, typename... clause_rest>
struct type_ladder_impl<idx, clause_head, clause_rest...> {
  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    bool eval_result = clause_head{}(field_list);
    if(eval_result) return idx;
    else return type_ladder_impl<idx + 1, clause_rest...>{}(field_list);
  }
};

template <typename clause_head, typename... clause_rest>
struct type_ladder<clause_head, clause_rest...> {
  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    return type_ladder_impl<0, clause_head, clause_rest...>{}(field_list);
  }
};

template <typename... clauses>
struct clauses_to_typelist;

template <typename... clauses>
struct clauses_to_typelist {
  using tlist = typelist::typelist<typename clauses::type_tag...>;
  // todo aargh, variable length types might have to computed at cast function
};

#endif // _COMPUTE_RES_
