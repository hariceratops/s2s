#ifndef _FIELD_NODE_HPP_
#define _FIELD_NODE_HPP_

#include <cstdint>
#include "../lib/metaprog/mp.hpp"

// todo better name
struct field_type_info {
  meta::type_identifier id;
  std::size_t occurs_at_idx;
};

#endif /* _FIELD_NODE_HPP_ */
