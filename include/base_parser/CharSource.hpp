#ifndef _CHAR_SOURCE_HPP
#define _CHAR_SOURCE_HPP

#include <stdexcept>
#include <string>
#include <string_view>

namespace base_parser {

struct CharSource {
  explicit CharSource(std::string_view view) noexcept;
  explicit CharSource(const std::string& str) noexcept;

  char next() noexcept;
  bool hasNext() const noexcept;

  std::runtime_error error() const noexcept;

private:
  const std::string_view data;
  std::size_t pos{0};
  std::size_t line_begin{0};
};

} // namespace base_parser

#endif
