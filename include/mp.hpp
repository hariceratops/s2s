#ifndef _MP_HPP_
#define _MP_HPP_

#include <cstdint>
#include <array>
#include <utility>


namespace meta {
using type_identifier = std::size_t;

namespace meta_impl {
// todo: fix warning due to friend injection
template <type_identifier>
struct type_id_key {
  constexpr auto friend get(type_id_key);
};

template <typename T>
struct type_id_value {
  using value_type = T;

  template <std::size_t left = 0u, std::size_t right = 1024u - 1u>
  static constexpr auto gen() -> std::size_t {
    if constexpr (left >= right) {
      constexpr bool exists = requires { get(type_id_key<type_identifier{left}>{}); };
      return (exists ? left + 1 : left);
    } else if constexpr (
      constexpr std::size_t mid = left + (right - left) / 2u; 
      requires { get(type_id_key<type_identifier{mid}>{}); }) 
    {
      return gen<mid + 1u, right>();
    } else {
      return gen<left, mid - 1u>();
    }
  }

  static constexpr auto id = type_identifier{gen()};

  constexpr auto friend get(type_id_key<id>) { 
    return type_id_value{}; 
  }
};
}

template<class T>
inline constexpr type_identifier type_id = meta_impl::type_id_value<T>::id;

template<type_identifier our_id>
using type_of = typename decltype(get(meta_impl::type_id_key<our_id>{}))::value_type;

static_assert(type_id<void> != type_id<int>);
static_assert(type_id<int> == type_id<int>);


template<class Fn, class T = decltype([]{})>
[[nodiscard]] inline constexpr auto invoke(Fn&& fn, type_identifier meta) {
  constexpr auto dispatch = [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
    return std::array{
      []<type_identifier N> {
        return +[](Fn fn) {
          if constexpr (requires { fn.template operator()<N>(); }) {
            return fn.template operator()<N>();
          }
        };
      }.template operator()<type_identifier{Ns}>()...
    };
  }(std::make_index_sequence<std::size_t{type_id<T>}>{});
  return dispatch[std::size_t{meta}](fn);
}

template <template<typename...> typename T, class... Ts, auto = []{}>
[[nodiscard]] inline constexpr auto invoke(type_identifier id) {
  return invoke([]<type_identifier id> {
    using type = type_of<id>;
    if constexpr(requires { T<Ts..., type>::value; }) {
      return T<Ts..., type>::value;
    } else {
      return type_id<typename T<Ts..., type>::type>;
    }
  }, id);
}

static_assert(invoke<std::is_const>(type_id<const int>));
static_assert(not invoke<std::is_const>(type_id<int>));
static_assert(type_id<int> == invoke<std::remove_pointer>(type_id<int*>));
}


#endif /* _MP_HPP_ */
