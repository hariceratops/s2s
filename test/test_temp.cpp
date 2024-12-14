
#include "../single_header/struct_cast.hpp"


auto main(void) -> int {
  using var_buffer_struct = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      str_field<"str", field_size<from_field<"len">>>
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
  static_assert(is_variable_sized_field_v<str_field<"str", field_size<from_field<"len">>>>);
  std::cout << std::string_view{fields["str"_f]};
};
