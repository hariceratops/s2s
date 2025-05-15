#ifndef _FIELD_NODE_HPP_
#define _FIELD_NODE_HPP_

#include <cstdint>
#include "mp.hpp"

// todo better name
struct field_node {
  meta::type_identifier id;
  std::size_t occurs_at_idx;
};

#endif /* _FIELD_NODE_HPP_ */
