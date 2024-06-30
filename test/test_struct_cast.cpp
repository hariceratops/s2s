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
void test_buffer_nested_read_TC01(void);
void test_fstream_nested_read_TC01(void);
void test_fixed_buffer_TC01(void);
void test_aliased_fixed_buffer_TC01(void);
void test_md_fixed_buffer_TC01(void);
void test_array_of_records_TC01(void);


// todo: Decide how to organise compile time tests
using test_struct_field_list = 
  struct_field_list<
    field<"a", u32, 4>, 
    field<"b", u32, 4>
  >;

using test_nested_struct_field_list = 
  struct_field_list<
    field<"a", u32, 4>, 
    field<"b", u32, 4>,
    struct_field<
      "c", 
      struct_field_list<
        field<"x", u32, 4>,
        field<"y", u32, 4>
      >
    >
  >;

using test_fixed_buffer_struct = 
  struct_field_list<
    field<"a", char[11], 11>,
    field<"b", fixed_string<11>, 11>,
    field<"c", std::array<int, 3>, 12>
  >;

using test_aliased_fixed_buffer_struct = 
  struct_field_list<
    c_str_field<"a", 10>,
    fixed_string_field<"b", 10>,
    fixed_array_field<"c", int, 3>
  >;


static_assert(std::is_same_v<field_lookup_v<to_field_list_v<test_struct_field_list>, field_accessor<"a">::field_id>, field<"a", u32, 4>>);
static_assert(std::is_same_v<field_lookup_v<to_field_list_v<test_struct_field_list>, field_accessor<"c">::field_id>, field_lookup_failed>);
static_assert(is_struct_field_list_v<test_struct_field_list>);
static_assert(!is_struct_field_list_v<field<"a", i32, 4>>);


auto main(void) -> int {
  test_buffer_read_TC01();
  test_fstream_read_TC01();
  test_buffer_nested_read_TC01();
  test_fstream_nested_read_TC01();
  test_fixed_buffer_TC01();
  test_aliased_fixed_buffer_TC01();
  test_md_fixed_buffer_TC01();
  test_array_of_records_TC01();

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


void test_buffer_nested_read_TC01(void) {
  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe,
    0xef, 0xbe, 0xad, 0xde
  };
  test_nested_struct_field_list sfl;

  struct_cast(sfl, buffer);
  auto a = sfl["a"_f];
  auto b = sfl["b"_f];
  auto x = sfl["c"_f]["x"_f];
  auto y = sfl["c"_f]["y"_f];

  assert(a == 0xdeadbeef);
  assert(b == 0xcafed00d);
  assert(x == 0xbeefbeef);
  assert(y == 0xdeadbeef);
}


void test_fstream_nested_read_TC01(void) {
  std::ofstream ofs("test_bin_input_2.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 x = 0xbeefbeef;
  u32 y = 0xdeadbeef;

  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&x), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&y), sizeof(a));
  ofs.close();

  std::ifstream ifs("test_bin_input_2.bin", std::ios::in | std::ios::binary);
  test_nested_struct_field_list sfl;
  struct_cast(sfl, ifs);
  ifs.close();

  auto expected_a = sfl["a"_f];
  auto expected_b = sfl["b"_f];
  auto expected_x = sfl["c"_f]["x"_f];
  auto expected_y = sfl["c"_f]["y"_f];

  assert(expected_a == 0xdeadbeef);
  assert(expected_b == 0xcafed00d);
  assert(expected_x == 0xbeefbeef);
  assert(expected_y == 0xdeadbeef);
}


void test_fixed_buffer_TC01(void) {
  std::ofstream ofs("test_bin_input_3.bin", std::ios::out | std::ios::binary);
  constexpr std::size_t str_len = 10;
  const u8 str[] = "foo in bar";
  const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_3.bin", std::ios::in | std::ios::binary);
  test_fixed_buffer_struct sfl;
  struct_cast(sfl, ifs);
  ifs.close();
  
  std::string_view expected{"foo in bar"};
  assert(std::string_view{sfl["a"_f]} == expected);
  assert(std::string_view{sfl["b"_f].data()} == expected);
  // todo assert array

  auto a = sfl["a"_f];
  auto b = sfl["b"_f];
  auto c = sfl["c"_f];
  std::cout << a << '\n';
  std::cout << b.data() << '\n';
  print_hex(c[0]);
  print_hex(c[1]);
  print_hex(c[2]);
};


void test_aliased_fixed_buffer_TC01(void) {
  std::ofstream ofs("test_bin_input_4.bin", std::ios::out | std::ios::binary);
  constexpr std::size_t str_len = 10;
  const u8 str[] = "foo in bar";
  const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_4.bin", std::ios::in | std::ios::binary);
  test_aliased_fixed_buffer_struct sfl;
  struct_cast(sfl, ifs);
  ifs.close();
  
  std::string_view expected{"foo in bar"};
  assert(std::string_view{sfl["a"_f]} == expected);
  assert(std::string_view{sfl["b"_f].data()} == expected);
  // todo assert array
  
  auto a = sfl["a"_f];
  auto b = sfl["b"_f];
  auto c = sfl["c"_f];
  std::cout << a << '\n';
  std::cout << b.data() << '\n';
  print_hex(c[0]);
  print_hex(c[1]);
  print_hex(c[2]);
};

void test_md_fixed_buffer_TC01(void) {
  using md_struct = 
    struct_field_list<
      fixed_array_field<"a", std::array<u32, 3>, 3>
    >;
  std::ofstream ofs("test_bin_input_5.bin", std::ios::out | std::ios::binary);
  const u32 u32_arr[3][3] = { 
    {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
    {0xdeadbeef, 0xcafed00d, 0xbeefbeef}, 
    {0xdeadbeef, 0xcafed00d, 0xbeefbeef} 
  };
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_5.bin", std::ios::in | std::ios::binary);
  md_struct sfl;
  struct_cast(sfl, ifs);
  ifs.close();
  
  // todo assert array
  
  auto a = sfl["a"_f];
  for(auto row: a) {
    for(auto cell: row) {
      print_hex(cell);
    }
  }
};

void test_array_of_records_TC01(void) {
  using test_struct = 
    struct_field_list <
      field<"a", u32, 4>,
      field<"b", u32, 4>
    >;
  using md_struct = 
    struct_field_list<
      fixed_array_field<"buffer", test_struct, 3>
    >;
  std::ofstream ofs("test_bin_input_6.bin", std::ios::out | std::ios::binary);
  const u32 u32_arr[3][2] = { 
    {0xdeadbeef, 0xbeefbeef},
    {0xdeadbeef, 0xbeefbeef}, 
    {0xdeadbeef, 0xbeefbeef} 
  };
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_6.bin", std::ios::in | std::ios::binary);
  md_struct sfl;
  struct_cast(sfl, ifs);
  ifs.close();
  
  // todo assert array
  
  auto buffer = sfl["buffer"_f];
  for(auto row: buffer) {
    print_hex(row["a"_f]);
    print_hex(row["b"_f]);
  }
};

