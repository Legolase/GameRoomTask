#include <base_parser/BaseParser.hpp>
#include <types/InputEvent.hpp>

#include <log.hpp>

namespace task {

namespace {

class EventParser : protected base_parser::BaseParser {
  using base = base_parser::BaseParser;

public:
  explicit EventParser(std::string_view view) :
      base(view)
  {}

  InputEvent parse()
  {
    InputEvent result = parseEvent();

    if (!end()) {
      throw error();
    }

    return result;
  }

private:
  InputEvent parseEvent()
  {
    InputEvent result;

    result.time = parseTime();
    expect(' ');
    result.type = parseType();
    expect(' ');
    result.client_id = parseClientID();

    if (result.type == InputEvent::Type::CLIENT_TAKE_TABLE) {
      expect(' ');
      result.table_id = getNumber() - 1;
    }

    return result;
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

  InputEvent::Type parseType()
  {
    int type = parseFixedLengthNumber(1);

    if (type < 1 || type > 4) {
      throw error();
    }

    return static_cast<InputEvent::Type>(type);
  }

  std::string parseClientID()
  {
    std::string result;

    while (!test(' ') && !end()) {
      if (between('a', 'z') || between('0', '9') || test('_') || test('-')) {
        result.push_back(take());
      } else {
        throw error();
      }
    }

    if (result.empty()) {
      throw error();
    }

    return result;
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

  uint getNumber()
  {
    std::string str_num;

    if (!between('1', '9')) {
      throw error();
    }

    str_num.push_back(take());

    while (between('0', '9')) {
      str_num.push_back(take());
    }

    try {
      return std::stoul(str_num);
    } catch (...) {
      throw error();
    }
  }
};

} // namespace

InputEvent InputEvent::get(std::string_view view)
{
  EventParser parser(view);

  return parser.parse();
}

} // namespace task
