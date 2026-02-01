#ifndef _REVENUER_HPP
#define _REVENUER_HPP

#include <types/InputEvent.hpp>

#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <sstream>
#include <string>

namespace task {

using ClientID = std::string;
using TableID = int;

class RevenuerManager {
  struct GeneratedEvent {
    enum class Type {
      CLIENT_LEAVE = 11,
      CLIENT_TAKE_TABLE,
      ERROR
    };

    int time;
    Type type;
    std::map<ClientID, TableID>::iterator client_it;
    int table_id;
    std::string error_message;
  };

  struct TableStatistic {
    uint revenue{0};
    uint used_time{0};
  };

public:
  RevenuerManager(std::istream& input_data, std::ostream& output_data) noexcept;

  RevenuerManager(const RevenuerManager&) = delete;
  RevenuerManager(RevenuerManager&&) = delete;

  void process();

private:
  void initialize();
  void finalize();

  void processGeneratedEvent(const GeneratedEvent& event);
  void processInputEvent(const InputEvent& event);

  void processClientArrive(const InputEvent& event);
  void processClientTakeTable(const InputEvent& event);
  void processClientWait(const InputEvent& event);
  void processClientLeave(const InputEvent& event);

  void setClientToTable(
      int current_time, std::map<ClientID, TableID>::iterator it, uint table_id
  );
  void unsetClientFromTable(int current_time, std::map<ClientID, TableID>::iterator it);
  void removeClient(int current_time, std::map<ClientID, TableID>::iterator it);
  void kickOutLeftClients();

  template<typename Exception, typename... Args>
  Exception error(Args&&... args)
  {
    std::stringstream stream;

    ((stream << "[ERROR] ") << ... << std::forward<Args>(args));

    return Exception(stream.str() + ".");
  }

  std::istream& in;
  std::stringstream prepared;
  std::ostream& out;

  std::vector<TableStatistic> table_staticstic_list;

  std::queue<GeneratedEvent> generated_event_queue;
  std::optional<InputEvent> deferred_event;

  std::map<ClientID, TableID> client2table;

  std::vector<int> table_time_busy;
  uint free_table_count;

  std::queue<ClientID> client_queue;

  int last_time_event{-1};

  int begin_time;
  int end_time;
  uint cost_per_hour;
};

} // namespace task

#endif
