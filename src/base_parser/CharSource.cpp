#include <base_parser/CharSource.hpp>

namespace base_parser {

CharSource::CharSource(std::string_view view) noexcept :
    data(view)
{}

CharSource::CharSource(const std::string& str) noexcept :
    data(str.data(), str.size())
{}

char CharSource::next() noexcept
{
  if (hasNext()) {
    if (pos > 0 && data[pos - 1] == '\n') {
      line_begin = pos;
    }
    return data[pos++];
  } else {
    return 0;
  }
}

bool CharSource::hasNext() const noexcept
{
  return pos < data.size();
}

std::runtime_error CharSource::error() const noexcept
{
  auto line_end{0};

  if (pos > 0) {
    line_end = pos - 1;
  }

  while (data[line_end] && data[line_end] != '\n') {
    ++line_end;
  }

  if (line_end - line_begin > 0) {
    return std::runtime_error(std::string(&(data[line_begin]), line_end - line_begin));
  }

  return std::runtime_error("");
}

} // namespace base_parser
