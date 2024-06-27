#include "../include/struct_cast.hpp"
#include "../include/value_constraints.hpp"
#include <iostream>
#include <cassert>


template <typename... ts>
struct to_field_list;

template <typename... ts>
struct to_field_list<struct_field_list<ts...>> {
  using type = field_list<ts...>;
};

template <typename T>
using to_field_list_v = typename to_field_list<T>::type;


void print_hex(int value) {
  std::cout << std::hex << "0x" << value << '\n';
}

auto main(void) -> int {
  using test_sfl = struct_field_list<
                    field<"a", int, 4>, 
                    field<"b", int, 4>
                   >;

  static_assert(std::is_same_v<field_lookup_v<to_field_list_v<test_sfl>, field_accessor<"a">::field_id>, field<"a", int, 4>>);
  static_assert(std::is_same_v<field_lookup_v<to_field_list_v<test_sfl>, field_accessor<"c">::field_id>, field_lookup_failed>);
  static_assert(is_struct_field_list_v<test_sfl>);
  static_assert(!is_struct_field_list_v<field<"a", int, 4>>);

  using u8 = const unsigned char;
  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };
  test_sfl sfl;
  struct_cast(sfl, buffer);
  assert(sfl["a"_f] = 0xdeadbeef);
  assert(sfl["b"_f] = 0xcafed00d);
  print_hex(sfl["a"_f]);
  print_hex(sfl["b"_f]);

  return 0;
}
