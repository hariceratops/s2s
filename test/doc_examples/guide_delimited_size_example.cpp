// Backs the until<> example in docs/schema/size-axis.md. The region between
// docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;

// An ELF string table is a run of NUL-terminated names with nothing else
// framing them; a symbol names its entry by a byte offset into this table, not
// by a length. Nothing on the wire says how long any one name is — the
// delimiter is the only thing that does, and max_bytes is the only bound on
// the search.
using elf_string_table_entry =
  s2s::struct_field_list<
    s2s::str_field<"name", s2s::until<u8{0}>, s2s::max_bytes<255>>
  >;

auto main() -> int {
  {
    std::ofstream out("strtab_entry.bin", std::ios::out | std::ios::binary | std::ios::trunc);
    out.write("_init\0", 6);
  }

  std::ifstream file("strtab_entry.bin", std::ios::in | std::ios::binary);

  const auto parsed =
    s2s::struct_cast_be<elf_string_table_entry>(file)
      .transform([](const elf_string_table_entry& entry) {
        return entry["name"_f] == "_init";
      });

  return parsed.value_or(false) ? 0 : 1;
}
// docs-end
