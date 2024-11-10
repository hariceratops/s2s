#ifndef _SIZE_DEDUCE_HPP_
#define _SIZE_DEDUCE_HPP_

#include "field_size.hpp"
#include "field_list.hpp"
#include "compute_res.hpp"
#include <utility>

template <typename T>
struct deduce_field_size;

template <std::size_t N>
struct deduce_field_size<field_size<fixed<N>>> {
  using field_size_type = field_size<fixed<N>>;

  constexpr auto operator()() -> std::size_t {
    return field_size_type::size;
  }
};

// Can work with from_field since from_field is an alias for runtime_size with single field dependency
// template <fixed_string id>
// using from_field = runtime_size<field_accessor<id>>;
template <fixed_string id>
struct deduce_field_size<field_size<runtime_size<field_accessor<id>>>> {
  using field_size_type = runtime_size<field_accessor<id>>;
  
  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& struct_fields) -> std::size_t {
    return struct_fields[field_size_type::accessor];
  }
};

template <auto callable, field_name_list req_fields>
struct deduce_field_size<field_size<size_from_fields<callable, req_fields>>> {
  using field_size_type = compute<callable, std::size_t, req_fields>;
  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& struct_fields) -> std::size_t {
    return field_size_type{}(struct_fields);
  }
};

// todo: decide on upcounting vs downcounting for this mess
template <std::size_t size_idx, atomic_size... sizes>
struct deduce_field_size_switch;

template <std::size_t size_idx>
struct deduce_field_size_switch<size_idx> {
  template <typename... fields>
  constexpr auto operator()(std::size_t size_idx_r, const struct_field_list<fields...>& struct_fields) -> std::size_t {
    std::unreachable();
  }
};

template <std::size_t size_idx, atomic_size head, atomic_size... tail>
struct deduce_field_size_switch<size_idx, field_size<size_choices<head, tail...>>> {
  template <typename... fields>
  constexpr auto operator()(std::size_t size_idx_r, const struct_field_list<fields...>& struct_fields) -> std::size_t {
    if(size_idx_r == size_idx) {
      if constexpr(comptime_size_like<head>) return deduce_field_size<head>{}();
      else deduce_field_size<head>{}(struct_fields);
    } else {
      deduce_field_size_switch<size_idx - 1, tail...>(size_idx_r, struct_fields);
    } 
  }
};

// todo meta function for size_choice count
template <atomic_size... sizes>
struct deduce_field_size<field_size<size_choices<sizes...>>> {
  constexpr static auto num_of_choices = sizeof...(sizes);

  template <typename... fields>
  constexpr auto operator()(std::size_t size_idx_r, const struct_field_list<fields...>& struct_fields) -> std::size_t {
    return deduce_field_size_switch<num_of_choices, sizes...>(size_idx_r, struct_fields);
  }
};

#endif // _SIZE_DEDUCE_HPP_
