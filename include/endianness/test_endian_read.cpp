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
  mem_rep<u32> be_obj(be_value, 4);
  mem_rep<u32> le_obj(le_value, 4);

  std::ifstream in_file("sample.bin", std::ios::binary);
  std::istreambuf_iterator<char> fstream_buf_iterator_be(in_file);

  std::copy_n(fstream_buf_iterator_be, 4, be_obj.rbegin());
  fstream_buf_iterator_be++;
  std::copy_n(fstream_buf_iterator_be, 4, le_obj.begin());

  std::cout << be_value << '\n';
  std::cout << le_value << '\n';

  in_file.close();

  return 0;
}
