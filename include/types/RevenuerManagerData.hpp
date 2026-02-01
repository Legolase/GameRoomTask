#ifndef REVENUER_MANAGER_DATA_HPP
#define REVENUER_MANAGER_DATA_HPP

#include <string_view>

namespace task {

struct RevenuerManagerData {
  unsigned int table_count;
  int begin_time;
  int end_time;
  unsigned int cost_per_hour;

  static RevenuerManagerData get(std::string_view view);
};

} // namespace task

#endif
