#include <string>
#include <expected>
#include <type_traits>
#include <fstream>
#include <iostream>
#include "endian_read.hpp"


struct host {};
struct foreign {};

struct little_endian {};
struct big_endian {};

using InputStreamIter = std::istreambuf_iterator<char>;

template <typename ObjIter>
constexpr auto copy_from_file(InputStreamIter& fstream_iter, 
                              ObjIter&& obj_byte_iter, 
                              std::size_t count) 
  -> std::expected<void, std::string>  
{
  std::size_t b_count = 0;
  auto end_of_stream = InputStreamIter{};

  while(b_count < count) {
    if(fstream_iter == end_of_stream) 
      return std::unexpected("eof");

    *obj_byte_iter = *fstream_iter;
    obj_byte_iter++;
    fstream_iter++;
    b_count++;
  }
  return {};
}


template <typename Endianness, typename T>
constexpr auto copy_from_file(InputStreamIter& fstream_iter, mem_rep<T>& obj) 
  -> std::expected<void, std::string>
{
  if constexpr(std::is_same_v<Endianness, little_endian>) {
    return copy_from_file(fstream_iter, obj.begin(), obj.size);
  } else if constexpr(std::is_same_v<Endianness, big_endian>) {
    return copy_from_file(fstream_iter, obj.rbegin(), obj.size);
  }
}


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
  std::istreambuf_iterator<char> fstream_buf_iterator(in_file);

  auto res_1 = copy_from_file<big_endian>(fstream_buf_iterator, be_obj);
  auto res_2 = copy_from_file<little_endian>(fstream_buf_iterator, le_obj);

  std::cout << be_value << '\n';
  std::cout << le_value << '\n';

  return 0;
}
