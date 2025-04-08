#include <cstring>
#include <cstdint>
#include <print>
#include <random>
#include <fstream>
#include <limits>


template <typename T>
T generate_random() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> dist(0, std::numeric_limits<T>::max());
  return dist(gen);
}

void generate_well_aligned() {
    auto member_1 = generate_random<uint32_t>();
    auto member_2 = generate_random<uint32_t>();
    auto member_3 = generate_random<uint32_t>();
    std::ofstream file("well_aligned.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&member_1), sizeof(member_1));
    file.write(reinterpret_cast<const char*>(&member_2), sizeof(member_2));
    file.write(reinterpret_cast<const char*>(&member_3), sizeof(member_3));
    std::println("{} {} {}", member_1, member_2, member_3);
}

void generate_padded() {
    auto member_1 = generate_random<uint32_t>();
    auto member_2 = generate_random<uint8_t>();
    auto member_3 = generate_random<uint32_t>();
    std::ofstream file("padded.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&member_1), sizeof(member_1));
    file.write(reinterpret_cast<const char*>(&member_2), sizeof(member_2));
    file.write(reinterpret_cast<const char*>(&member_3), sizeof(member_3));
    std::println("{} {} {}", member_1, member_2, member_3);
}

void generate_well_aligned_shorter() {
    auto member_1 = generate_random<uint32_t>();
    auto member_2 = generate_random<uint32_t>();
    auto member_3 = generate_random<uint32_t>();
    std::ofstream file("well_aligned_shorter.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&member_1), sizeof(member_1));
    file.write(reinterpret_cast<const char*>(&member_2), 3);
    file.write(reinterpret_cast<const char*>(&member_3), sizeof(member_3));
    std::println("{} {} {}", member_1, member_2, member_3);
}

struct some_thing_well_aligned {
    uint32_t member_1;
    uint32_t member_2;
    uint32_t member_3;
};

struct some_thing_with_padding {
    uint32_t member_1;
    uint8_t member_2;
    uint32_t member_3;
};

struct some_thing_well_aligned_but_has_shorter_field {
    uint32_t member_1;
    uint32_t member_2;
    uint32_t member_3;
};


auto main(void) -> int {
    std::println("sizeof(some_thing_well_aligned) = {}", sizeof(some_thing_well_aligned));
    std::println("sizeof(some_thing_with_padding) = {}", sizeof(some_thing_with_padding));
    std::println("sizeof(some_thing_well_aligned_but_has_shorter_field) = {}", sizeof(some_thing_well_aligned_but_has_shorter_field));

    generate_well_aligned();
    std::ifstream well_aligned_file("well_aligned.bin", std::ios::binary | std::ios::in);
    some_thing_well_aligned aligned_instance;
    well_aligned_file.read(reinterpret_cast<char*>(&aligned_instance), sizeof(aligned_instance));
    std::println("{} {} {}", aligned_instance.member_1, aligned_instance.member_2, aligned_instance.member_3);
    well_aligned_file.close();

    generate_padded();
    std::ifstream padded_file("padded.bin", std::ios::binary | std::ios::in);
    some_thing_with_padding padded_instance;
    padded_file.read(reinterpret_cast<char*>(&padded_instance), sizeof(padded_instance));
    std::println("{} {} {}", padded_instance.member_1, padded_instance.member_2, padded_instance.member_3);
    padded_file.close();

    generate_well_aligned_shorter();
    std::ifstream well_aligned_shorter_file("well_aligned_shorter.bin", std::ios::binary | std::ios::in);
    some_thing_well_aligned_but_has_shorter_field shorter_instance;
    well_aligned_shorter_file.read(reinterpret_cast<char*>(&shorter_instance), sizeof(shorter_instance));
    std::println("{} {} {}", shorter_instance.member_1, shorter_instance.member_2, shorter_instance.member_3);
    well_aligned_shorter_file.close();

    return 0;
}
