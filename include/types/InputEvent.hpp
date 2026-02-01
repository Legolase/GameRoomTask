#ifndef _EVENT_HPP
#define _EVENT_HPP

#include <iostream>
#include <string>

namespace task {

struct InputEvent {
  enum class Type {
    CLIENT_ARRIVE = 1,
    CLIENT_TAKE_TABLE,
    CLIENT_WAIT,
    CLIENT_LEAVE,
  };

  static InputEvent get(std::string_view view);

  int time;
  Type type;
  std::string client_id;
  uint table_id;
};

} // namespace task

#endif
