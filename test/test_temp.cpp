#include <iostream>
#include "../single_header/struct_cast.hpp"


auto main(void) -> int {
  // using u32 = unsigned int;
  // using test_struct_field_list = 
  //  struct_field_list<
  //    basic_field<"a", u32, field_size<fixed<4>>>, 
  //    basic_field<"b", u32, field_size<fixed<4>>>
  // >;
  //
  // std::ofstream ofs("test_bin_input_1.bin", std::ios::out | std::ios::binary);
  // u32 a = 0xdeadbeef;
  // u32 b = 0xcafed00d;
  // ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  // ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  // ofs.close();
  //
  // std::ifstream ifs("test_bin_input_1.bin", std::ios::in | std::ios::binary);
  //
  // auto result = struct_cast<test_struct_field_list>(ifs);
  // ifs.close();
  // if(!result) std::cout << static_cast<int>(result.error());
  // else {
  //   auto fields = *result;
  //   std::cout << fields["a"_f] << " " << fields["b"_f] << '\n';
  // }

  using var_buffer_struct = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      str_field<"str", field_size<len_from_field<"len">>>
    >;

  constexpr std::size_t str_len = 10;
  const unsigned char str[] = "foo in bar";

  std::ofstream ofs("test_bin_input_str_fields.bin", std::ios::out | std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
  ofs.write(reinterpret_cast<const char*>(&str), str_len);
  ofs.close();

  std::ifstream ifs("test_bin_input_str_fields.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<var_buffer_struct>(ifs);
  ifs.close();

  auto fields = *res;
  static_assert(is_variable_sized_field_v<str_field<"str", field_size<len_from_field<"len">>>>);
  std::cout << fields["len"_f] << " " << fields["str"_f].size() <<  std::string_view{fields["str"_f]};
};
