#include <iostream>

// Helper metafunctions
template <typename... ts>
struct to_field_list;

template <typename... ts>
struct to_field_list<struct_field_list<ts...>> {
  using type = field_list<ts...>;
};

template <typename T>
using to_field_list_v = typename to_field_list<T>::type;


static inline void print_hex(int value) {
  std::cout << std::hex << "0x" << value << '\n';
}
