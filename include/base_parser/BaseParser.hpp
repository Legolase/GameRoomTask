#ifndef _BASE_PARSER_HPP
#define _BASE_PARSER_HPP

#include <base_parser/CharSource.hpp>

namespace base_parser {

class BaseParser {
  static constexpr char END = 0;

public:
  explicit BaseParser(std::string_view view) noexcept;
  explicit BaseParser(const std::string& str) noexcept;

  char current() const noexcept;
  bool test(char value) const noexcept;
  char end() const noexcept;
  char take() noexcept;
  bool take(char value) noexcept;
  void expect(char value);

  bool between(char min, char max) const noexcept;

  std::runtime_error error() const noexcept;

private:
  CharSource source;
  char current_;
};

} // namespace base_parser

#endif
