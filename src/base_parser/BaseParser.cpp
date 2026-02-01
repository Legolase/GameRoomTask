#include <base_parser/BaseParser.hpp>

namespace base_parser {
BaseParser::BaseParser(std::string_view view) noexcept :
    source(view)
{
  take();
}

BaseParser::BaseParser(const std::string& str) noexcept :
    source(str)
{
  take();
}

char BaseParser::current() const noexcept
{
  return current_;
}

bool BaseParser::test(char value) const noexcept
{
  return current() == value;
}

char BaseParser::end() const noexcept
{
  return test(END);
}

char BaseParser::take() noexcept
{
  auto result = current_;
  current_ = source.next();

  return result;
}

bool BaseParser::take(char value) noexcept
{
  if (test(value)) {
    take();
    return true;
  }
  return false;
}

void BaseParser::expect(char value)
{
  if (!take(value)) {
    if (!end()) {
      throw error();
    }
  }
}

bool BaseParser::between(char min, char max) const noexcept
{
  return min <= current() && current() <= max;
}

std::runtime_error BaseParser::error() const noexcept
{
  return source.error();
}

} // namespace base_parser
