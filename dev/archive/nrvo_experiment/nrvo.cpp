#include <expected>
#include <cstdio>
#include <vector>

struct counter {
  static inline int copies = 0, moves = 0;
  std::vector<int> payload;
  counter() = default;
  counter(const counter& o) : payload(o.payload) { ++copies; }
  counter(counter&& o) noexcept : payload(std::move(o.payload)) { ++moves; }
  counter& operator=(const counter& o) { payload = o.payload; ++copies; return *this; }
  counter& operator=(counter&& o) noexcept { payload = std::move(o.payload); ++moves; return *this; }
};

using R = std::expected<counter, int>;

bool read_into(counter& c, bool fail) {
  c.payload.assign(1000, 7);
  return !fail;
}

// Today's shape: a local S, then R(S) on the way out.
R today(bool fail) {
  counter field_list;
  bool ok = read_into(field_list, fail);
  return ok ? R(field_list) : std::unexpected(1);
}

// Proposed shape: one named R, read into its value, error assigned into it.
R in_place(bool fail) {
  R r{std::in_place};
  if (!read_into(*r, fail))
    r = std::unexpected(1);
  return r;
}

template <auto f>
void run(const char* name, bool fail) {
  counter::copies = counter::moves = 0;
  R r = f(fail);
  std::printf("%-9s %-7s copies=%d moves=%d has_value=%d\n", name, fail ? "fail" : "success",
              counter::copies, counter::moves, r.has_value());
}

int main() {
  run<today>("today", false);
  run<today>("today", true);
  run<in_place>("in_place", false);
  run<in_place>("in_place", true);
}
