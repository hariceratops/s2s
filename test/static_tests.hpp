
static_assert(type_id<void> != type_id<int>);
static_assert(type_id<int> == type_id<int>);
static_assert(invoke<std::is_const>(type_id<const int>));
static_assert(not invoke<std::is_const>(type_id<int>));
static_assert(type_id<int> == invoke<std::remove_pointer>(type_id<int*>));

