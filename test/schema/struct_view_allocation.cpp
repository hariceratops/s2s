#include <gtest/gtest.h>
#include <array>
#include <cstdlib>
#include <new>
#include <span>
#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

// Replacing the global allocation functions affects the whole binary, which is
// why this lives apart from struct_view_read.cpp. Counting is gated so gtest's
// own allocations never reach the counter.
namespace {
std::size_t allocations = 0;
bool counting = false;
}

void* operator new(std::size_t size) {
  if(counting)
    ++allocations;
  if(void* p = std::malloc(size ? size : 1))
    return p;
  throw std::bad_alloc{};
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

namespace {
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using entry =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B, s2s::gte(0u)>,
    s2s::basic_field<"flags", u16, 2_B>
  >;
using inner =
  s2s::struct_field_list<
    s2s::basic_field<"x", u8, 1_B>,
    s2s::basic_field<"y", u32, 4_B>
  >;
using schema =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::basic_field<"wide", u32, 4_B>,
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::fixed_string_field<"name", 4>,
    s2s::struct_field<"in", inner>,
    s2s::array_of_records<"entries", entry, 3>
  >;
constexpr std::size_t width = 1 + 4 + 6 + 5 + 5 + 21;
}

TEST(StructViewAllocation, CreationAndReadsAllocateNothing) {
  static_assert(s2s::wire_width<schema> == width);
  std::array<char, width + 1> storage{};
  for(std::size_t idx = 1; idx < storage.size(); ++idx)
    storage[idx] = static_cast<char>(idx);
  std::span<const char> bytes{storage.data() + 1, width};

  u32 wide_le = 0, wide_be = 0, y = 0, len = 0;
  u16 element = 0;
  bool created = false;

  allocations = 0;
  counting = true;
  {
    auto le = s2s::struct_view_le<schema>(bytes);
    auto be = s2s::struct_view_be<schema>(bytes);
    created = le.has_value() && be.has_value();
    if(created) {
      wide_le = (*le)["wide"_f];
      wide_be = (*be)["wide"_f];
      element = (*le)["arr"_f][2];
      y = (*le)["in"_f]["y"_f];
      len = (*be)["entries"_f][2]["len"_f];
    }
    auto name = (*le)["name"_f];
    (void)name;
  }
  counting = false;

  EXPECT_EQ(allocations, 0u);
  ASSERT_TRUE(created);
  EXPECT_NE(wide_le, wide_be);
  EXPECT_NE(element, 0u);
  EXPECT_NE(y, 0u);
  EXPECT_NE(len, 0u);
}

TEST(StructViewAllocation, TheCounterSeesAnAllocation) {
  allocations = 0;
  counting = true;
  void* p = ::operator new(sizeof(int));
  asm volatile("" : : "r"(p) : "memory");
  counting = false;
  ::operator delete(p);
  EXPECT_GE(allocations, 1u);
}
