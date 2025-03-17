#include <expected>
#include <fstream>
#include <iostream>
#include "endian_read.hpp"


auto main(void) -> int {
  using u32 = unsigned int;

  u32 value = 0xdeadbeef;
  std::ofstream out_file("sample.bin", std::ios::binary);
  out_file.write(reinterpret_cast<char*>(&value), sizeof(value));
  out_file.write(reinterpret_cast<char*>(&value), sizeof(value));
  out_file.close();

  u32 be_value = 0;
  u32 le_value = 0;
  raw_bytes<u32> be_obj(be_value, 4);
  raw_bytes<u32> le_obj(le_value, 4);

  std::ifstream in_file("sample.bin", std::ios::binary);
  std::istreambuf_iterator<char> fstream_buf_iterator(in_file);

  auto res_1 = copy_from_file<std::endian::big>(fstream_buf_iterator, be_obj);
  auto res_2 = copy_from_file<std::endian::little>(fstream_buf_iterator, le_obj);

  std::cout << be_value << '\n';
  std::cout << le_value << '\n';

  return 0;
}
