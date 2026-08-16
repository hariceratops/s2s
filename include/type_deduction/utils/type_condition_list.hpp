#ifndef _TYPE_CONDITION_LIST_HPP_
#define _TYPE_CONDITION_LIST_HPP_


namespace s2s {
// Deliberately includes nothing and constrains nothing. field.hpp specializes
// to_field_choices on this, and its include closure is three headers deep;
// naming type_switch/type_if_else there instead would drag the whole
// type_deduction subtree under the most-included header in the project. The
// pack is unconstrained for the same reason — type_condition_like lives in
// helper.hpp. Nothing is lost, since only type_switch and type_if_else ever
// build one of these, and both already constrain their own packs.
template <typename... cases>
struct type_condition_list {};
} /* namespace s2s */

#endif // _TYPE_CONDITION_LIST_HPP_
