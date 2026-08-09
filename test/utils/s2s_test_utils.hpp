#ifndef _S2S_TEST_UTILS_HPP_
#define _S2S_TEST_UTILS_HPP_


#include <fstream>
#include <sstream>
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

// Round-trip: write a populated struct to an in-memory stream, read it back,
// and expose both `written` (the write result) and `result` (the read-back
// result) to the checking code. std::stringstream satisfies both
// output_stream_like and input_stream_like.
#define FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, code) \
  do { \
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary); \
    auto written = s2s::struct_write_le<test_field_list>(stream, obj); \
    auto result = s2s::struct_cast_le<test_field_list>(stream); \
    code; \
  } while(0)

#define FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, code) \
  do { \
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary); \
    auto written = s2s::struct_write_be<test_field_list>(stream, obj); \
    auto result = s2s::struct_cast_be<test_field_list>(stream); \
    code; \
  } while(0)

// For the rejection-path tests: assert the write failed with a given reason
// and that the struct never reached the stream.
#define FIELD_LIST_LE_WRITE_REJECTED(obj, reason) \
  do { \
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary); \
    auto written = s2s::struct_write_le<test_field_list>(stream, obj); \
    ASSERT_FALSE(written.has_value()); \
    EXPECT_EQ(written.error().failure_reason, reason); \
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
