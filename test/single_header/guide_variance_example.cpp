// Backs the variance/type-deduction example in UserGuide.md. The region between
// docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>
#include <variant>

using namespace s2s_literals;

using u32 = unsigned int;
using i32 = int;

// Selection by discriminant: "tag" says which alternative "body" holds.
using switched =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>>,
          s2s::match_case<2, s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>>
        >
      >
    >
  >;

// Selection by predicate: no discriminant on the wire at all, so there is no
// match_field and nothing for the library to derive.
constexpr auto sum_is_small = [](auto a, auto b) { return a + b < 100u; };
constexpr auto sum_is_large = [](auto a, auto b) { return a + b >= 100u; };

using laddered =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<sum_is_small, s2s::with_fields<"a", "b">>,
            s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::branch<
            s2s::predicate<sum_is_large, s2s::with_fields<"a", "b">>,
            s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >
  >;

auto main() -> int {
  // "tag" is never assigned: it is derived from which alternative body holds.
  switched s{};
  s["body"_f] = i32{-7};

  std::stringstream a(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<switched>(a, s); !written)
    return 1;
  const auto s_back = s2s::struct_cast_be<switched>(a);
  if(!s_back || (*s_back)["tag"_f] != 2 || std::get<i32>((*s_back)["body"_f]) != -7)
    return 1;

  // "a" and "b" stay assignable: a predicate has no inverse, so they are
  // verified against the alternative held rather than derived from it.
  laddered l{};
  l["a"_f] = 10u;
  l["b"_f] = 20u;
  l["body"_f] = u32{42};

  std::stringstream b(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<laddered>(b, l); !written)
    return 1;
  const auto l_back = s2s::struct_cast_be<laddered>(b);
  if(!l_back || std::get<u32>((*l_back)["body"_f]) != 42)
    return 1;

  // The held alternative contradicts the branch the predicates select.
  laddered wrong{};
  wrong["a"_f] = 500u;
  wrong["b"_f] = 500u;
  wrong["body"_f] = u32{42};
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<laddered>(discard, wrong);

  return !rejected
      && rejected.error().failure_reason == s2s::error_reason::validation_failure
        ? 0 : 1;
}
// docs-end
