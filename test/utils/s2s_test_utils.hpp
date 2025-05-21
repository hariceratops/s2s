#ifndef _S2S_TEST_UTILS_HPP_
#define _S2S_TEST_UTILS_HPP_


#include <fstream>
#include "../../single_header/s2s.hpp"


#define PREPARE_INPUT_FILE(code) \
  []() { \
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary); \
    code; \
  }()


#define FIELD_LIST_SCHEMA using test_field_list

#define FIELD_LIST_LE_READ_CHECK(code) \
  do { \
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary); \
    auto result = s2s::struct_cast_le<test_field_list>(file); \
    code; \
  } while(0)

#define FIELD_LIST_BE_READ_CHECK(code) \
  do { \
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary); \
    auto result = s2s::struct_cast_be<test_field_list>(file); \
    code; \
  } while(0)

// constexpr auto operator""_field_list_be_read_check(auto callable) {
//   callable();
// }

template <typename struct_field_list>
struct field_list_le_read_check {
  using callable = void(*)(void);
  callable fn;

  template <typename Fn>
  constexpr field_list_le_read_check(Fn&& fn): fn(+fn) {}

  constexpr void operator()() { 
    fn(); 
  }
};


// Helper types
using i32 = int;
using u32 = unsigned int;
using u8 = unsigned char;
using u16 = unsigned short;

#endif /* _S2S_TEST_UTILS_HPP_ */
