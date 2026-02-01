#include <base_parser/BaseParser.hpp>
#include <types/RevenuerManagerData.hpp>

namespace task {

namespace {

class RevenuerManagerDataParser : protected base_parser::BaseParser {
  using base = base_parser::BaseParser;

public:
  RevenuerManagerDataParser(std::string_view view) :
      base(view)
  {}

  RevenuerManagerData parse()
  {
    RevenuerManagerData result;

    result.table_count = parseUnsignedInt();

    if (result.table_count < 1) {
      throw error();
    }

    expect('\n');
    result.begin_time = parseTime();
    expect(' ');
    result.end_time = parseTime();
    expect('\n');
    result.cost_per_hour = parseUnsignedInt();

    if (result.cost_per_hour == 0) {
      throw error();
    }

    return result;
  }

private:
  unsigned int parseUnsignedInt()
  {
    std::string str_num;

    if (!between('0', '9')) {
      throw error();
    }

    str_num.push_back(take());

    while (between('0', '9')) {
      str_num.push_back(take());
    }

    try {
      return std::stoul(str_num);
    } catch (...) {
      throw std::runtime_error("Invalid unsigned int was parsed.");
    }
  }

  int parseTime()
  {
    int hour = parseHour();
    expect(':');
    int minute = parseMinute();

    return hour * 60 + minute;
  }

  int parseHour()
  {
    int hour = parseFixedLengthNumber(2);

    if (hour < 0 || hour > 23) {
      throw error();
    }

    return hour;
  }

  int parseMinute()
  {
    int hour = parseFixedLengthNumber(2);

    if (hour < 0 || hour > 59) {
      throw error();
    }

    return hour;
  }

  int parseFixedLengthNumber(std::size_t length)
  {
    std::string str_num;
    for (std::size_t i = 0; i < length; ++i) {
      if (!between('0', '9')) {
        throw error();
      }
      str_num.push_back(take());
    }

    try {
      return std::stoi(str_num);
    } catch (...) {
      throw error();
    }
  }
};

} // namespace

RevenuerManagerData RevenuerManagerData::get(std::string_view view)
{
  RevenuerManagerDataParser parser(view);

  return parser.parse();
}

} // namespace task
