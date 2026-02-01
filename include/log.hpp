#ifndef _DEFINES_HPP
#define _DEFINES_HPP

#include <iomanip>
#include <iostream>
#include <string.h>
#include <type_traits>
#include <vector>

namespace defines_details {

struct LogStreamer;

LogStreamer log_info_impl(std::ostream& out = std::cout);
LogStreamer log_debug_impl(std::ostream& out = std::cout);
LogStreamer log_warning_impl(std::ostream& out = std::cout);
LogStreamer log_error_impl(std::ostream& out = std::cerr);

struct LogStreamer {
  explicit LogStreamer(std::ostream& out_) noexcept;
  ~LogStreamer();

  LogStreamer(const LogStreamer& other) = delete;
  LogStreamer(LogStreamer&& other) noexcept;

  LogStreamer& operator=(const LogStreamer& other) = delete;
  LogStreamer& operator=(LogStreamer&& other) noexcept = delete;

  template<typename T>
  LogStreamer& operator<<(T&& value)
  {
    out << std::forward<T>(value);

    return *this;
  }

private:
  friend LogStreamer log_info_impl(std::ostream&);
  friend LogStreamer log_debug_impl(std::ostream&);
  friend LogStreamer log_warning_impl(std::ostream&);
  friend LogStreamer log_error_impl(std::ostream&);

  LogStreamer(LogStreamer& other) noexcept;

  std::ostream& out;
  bool holded{true};
};

struct NullLogStreamer {
  template<typename T>
  NullLogStreamer operator<<(T&& value)
  {
    return *this;
  }
};

static_assert(std::is_copy_assignable_v<LogStreamer> == false);
static_assert(std::is_copy_constructible_v<LogStreamer> == false);
static_assert(std::is_move_assignable_v<LogStreamer> == false);
static_assert(std::is_move_constructible_v<LogStreamer>);

} // namespace defines_details

#define LOG_INFO(...) (defines_details::log_info_impl(__VA_ARGS__))
#ifndef NDEBUG
#define LOG_DEBUG(...) (defines_details::log_debug_impl(__VA_ARGS__))
#else
#define LOG_DEBUG(...) (defines_details::NullLogStreamer{})
#endif
#define LOG_WARNING(...) (defines_details::log_warning_impl(__VA_ARGS__))
#define LOG_ERROR(...) (defines_details::log_error_impl(__VA_ARGS__))

#endif
