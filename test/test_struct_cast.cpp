#include "../include/struct_cast.hpp"
#include "../include/value_constraints.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cassert>


// Helper types
using i32 = int;
using u32 = unsigned int;
using u8 = unsigned char;

// Helper metafunctions
template <typename... ts>
struct to_field_list;

template <typename... ts>
struct to_field_list<struct_field_list<ts...>> {
  using type = field_list<ts...>;
};

template <typename T>
using to_field_list_v = typename to_field_list<T>::type;

// Helper functions
static inline void print_hex(int value) {
  std::cout << std::hex << "0x" << value << '\n';
}


void test_buffer_read_TC01(void);
void test_fstream_read_TC01(void);


// todo: Decide how to organise compile time tests
using test_struct_field_list = 
  struct_field_list<
    field<"a", u32, 4>, 
    field<"b", u32, 4>
  >;

static_assert(std::is_same_v<field_lookup_v<to_field_list_v<test_struct_field_list>, field_accessor<"a">::field_id>, field<"a", u32, 4>>);
static_assert(std::is_same_v<field_lookup_v<to_field_list_v<test_struct_field_list>, field_accessor<"c">::field_id>, field_lookup_failed>);
static_assert(is_struct_field_list_v<test_struct_field_list>);
static_assert(!is_struct_field_list_v<field<"a", i32, 4>>);


auto main(void) -> int {
  test_buffer_read_TC01();
  test_fstream_read_TC01();
  return 0;
}


void test_buffer_read_TC01(void) {
  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };
  test_struct_field_list sfl;

  struct_cast(sfl, buffer);
  auto a = sfl["a"_f];
  auto b = sfl["b"_f];

  assert(a == 0xdeadbeef);
  assert(b == 0xcafed00d);
}

void test_fstream_read_TC01(void) {
  std::ofstream ofs("test_bin_input_1.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(a));
  ofs.close();

  std::ifstream ifs("test_bin_input_1.bin", std::ios::in | std::ios::binary);
  test_struct_field_list sfl;
  struct_cast(sfl, ifs);
  ifs.close();

  auto expected_a = sfl["a"_f];
  auto expected_b = sfl["b"_f];

  assert(expected_a == 0xdeadbeef);
  assert(expected_b == 0xcafed00d);
}
