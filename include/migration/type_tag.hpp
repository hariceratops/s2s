#ifndef _TYPE_TAG_HPP_
#define _TYPE_TAG_HPP_


// todo is this required
// todo constraint T and size
template <typename T, typename size>
struct type_tag {
  using type = T;
  using field_size = size;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<type_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_type_tag_v = is_type_tag<T>::res;


#endif // _TYPE_TAG_HPP_
