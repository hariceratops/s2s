#ifndef _BIT_HPP_
#define _BIT_HPP_

// todo check if this handrolled implementation is required
// template <std::integral T>
// constexpr auto byteswap(T value) -> T {
//   constexpr auto object_size = sizeof(T);
//   auto value_rep = std::bit_cast<std::array<std::byte, object_size>>(value);
//   for(std::size_t fwd_idx = 0, rev_idx = object_size - 1; 
//       fwd_idx <= rev_idx; 
//       ++fwd_idx, --rev_idx) 
//   {
//     auto tmp = value_rep[fwd_idx];
//     value_rep[fwd_idx] = value_rep[rev_idx];
//     value_rep[rev_idx] = tmp;
//   }
//   return std::bit_cast<T>(value_rep);
// }
//
//
// constexpr auto is_little() -> bool {
//   constexpr uint32_t bait = 0xdeadbeef;
//   constexpr auto bait_size = sizeof(bait);
//   auto value_rep = std::bit_cast<std::array<std::byte, bait_size>>(bait);
//   return value_rep[0] == std::byte{0xef};
// }
//
// static_assert(byteswap(0xdeadbeef) == 0xefbeadde);
//
//
// enum endian: uint32_t {
//   little = 0xdeadbeef,
//   big = 0xefbeadde,
//   native = is_little() ? little : big
// };

#endif // _BIT_HPP_
