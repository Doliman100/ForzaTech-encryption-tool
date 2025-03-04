#include <ranges>
#include "iobinary.h"

std::ostream &PrintBlock(std::ostream &os, std::array<uint8_t, 16> &data) {
  return os << (data
    | std::views::transform([](uint8_t v) { return std::format("{:02X}", v); })
    | std::views::join_with(' ')
    | std::ranges::to<std::string>());
}
