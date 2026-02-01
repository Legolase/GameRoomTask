#include <iomanip>
#include <log.hpp>

namespace defines_details {

LogStreamer::LogStreamer(std::ostream& out_) noexcept :
    out(out_)
{}

LogStreamer::~LogStreamer()
{
  if (holded) {
    out << std::endl;
  }
}

LogStreamer::LogStreamer(LogStreamer& other) noexcept :
    out(other.out)
{
  other.holded = false;
}

LogStreamer::LogStreamer(LogStreamer&& other) noexcept :
    out(other.out)
{
  other.holded = false;
}

LogStreamer log_info_impl(std::ostream& out)
{
  LogStreamer streamer(out);
  return streamer << "   [INFO] ";
}

LogStreamer log_debug_impl(std::ostream& out)
{
  LogStreamer streamer(out);
  return streamer << "  [DEBUG] ";
}

LogStreamer log_warning_impl(std::ostream& out)
{
  LogStreamer streamer(out);
  return streamer << "[WARNING] ";
}

LogStreamer log_error_impl(std::ostream& out)
{
  LogStreamer streamer(out);
  return streamer << "  [ERROR] ";
}

} // namespace defines_details
