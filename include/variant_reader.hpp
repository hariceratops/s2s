#include <variant>
#include <sstream>
#include <iostream>
#include <expected>

// Error codes
enum class CastError {
  BUFFER_EXHAUSTION = 0,
  TYPE_DEDUCTION_FAILURE = 1
};


// Custom pipeline operator to propagate expected errors
template<typename T, typename F>
auto operator|(std::expected<T, CastError>&& exp, F&& func) {
  return exp ? func(exp) : exp;
}

// Helper function to read bytes into the variant
template<std::size_t idx, typename T>
struct variant_read_helper {
  std::stringstream& s;
  std::size_t size_to_read;
  std::size_t idx_r;

  constexpr explicit variant_read_helper(
    std::stringstream& s, 
    std::size_t size_to_read, 
    std::size_t idx_r) :
      s(s), size_to_read(size_to_read), idx_r(idx_r) {}

  constexpr auto operator()(std::expected<T, CastError>& exp) -> std::expected<T, CastError> {
    if (idx_r != idx) 
      return exp;
    typename std::variant_alternative_t<idx, T> temp{};
    if (!s.read(reinterpret_cast<char*>(&temp), size_to_read)) {
      return std::unexpected(CastError::BUFFER_EXHAUSTION);
    }
    exp = temp;
    return exp;
  }
};


// Main function to read variant
template<typename T, std::size_t... idx>
constexpr auto read_variant(std::stringstream& s, std::size_t size_to_read, std::size_t idx_r) 
    -> std::expected<T, CastError>
{
  if(idx_r > std::variant_size_v<T>)
    return std::unexpected(CastError::TYPE_DEDUCTION_FAILURE);
    
  return (
    std::expected<T, CastError>{} | 
    ... | 
    variant_read_helper<idx, T>(s, size_to_read, idx_r)
  );
}

inline int _main() {
  using MyVariant = std::variant<unsigned int, float, double>;

  // std::ifstream ifs("data.bin", std::ios::binary);
  std::stringstream s;
  unsigned int a = 0xdeadbeef;
  s.write(reinterpret_cast<const char*>(&a), sizeof(unsigned int));

  auto result = read_variant<MyVariant, 0, 1, 2>(s, sizeof(unsigned int), 0);
  if (result) {
    std::visit([](auto&& val) {
      std::cout << "value = " << val << std::endl;
    }, *result);
  } else {
    std::cout << "error = " << static_cast<unsigned int>(result.error()) << std::endl;
  }

  auto err = read_variant<MyVariant, 0, 1, 2>(s, sizeof(unsigned int), 5);
  if (err) {
    std::visit([](auto&& val) {
      std::cout << "value = " << val << std::endl;
    }, *err);
  } else {
    std::cout << "error = " << static_cast<unsigned int>(err.error()) << std::endl;
  }

  return 0;
}
