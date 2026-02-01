#include <RevenuerManager.hpp>

#include <base_parser/BaseParser.hpp>
#include <types/RevenuerManagerData.hpp>

#include <iomanip>
#include <log.hpp>
#include <sstream>

namespace {

std::string formatTime(int time)
{
  std::stringstream stream;
  stream << std::setfill('0');

  auto hour = time / 60;
  auto minute = time % 60;

  stream << std::setw(2) << hour;
  stream << ":";
  stream << std::setw(2) << minute;

  return stream.str();
}

std::string to_string(const task::InputEvent& event)
{
  using namespace task;
  std::stringstream stream;

  stream << formatTime(event.time) << ' ' << static_cast<int>(event.type) << ' ';

  switch (event.type) {
  case InputEvent::Type::CLIENT_ARRIVE:
  case InputEvent::Type::CLIENT_WAIT:
  case InputEvent::Type::CLIENT_LEAVE: {
    stream << event.client_id;
    break;
  }
  case InputEvent::Type::CLIENT_TAKE_TABLE: {
    stream << event.client_id << ' ' << event.table_id + 1;
    break;
  }
  }

  return stream.str();
}

} // namespace

namespace task {

RevenuerManager::RevenuerManager(
    std::istream& input_data, std::ostream& output_data
) noexcept :
    in(input_data),
    out(output_data)
{}

void RevenuerManager::process()
{
  initialize();

  while (true) {
    if (!generated_event_queue.empty()) {
      processGeneratedEvent(generated_event_queue.front());
      generated_event_queue.pop();
      continue;
    }

    if (deferred_event.has_value()) {
      auto event = std::move(deferred_event.value());
      deferred_event.reset();
      try {
        processInputEvent(event);
      } catch (...) {
        throw std::runtime_error(to_string(event));
      }
      continue;
    }

    std::string event_str;

    std::getline(in, event_str);

    if (event_str.empty()) {
      if (client2table.size()) {
        kickOutLeftClients();
        continue;
      }
      break;
    }

    prepared << event_str << std::endl;

    auto event = InputEvent::get(event_str);

    try {
      processInputEvent(event);
    } catch (...) {
      throw std::runtime_error(event_str);
    }
  }

  finalize();
}

void RevenuerManager::initialize()
{
  std::string init_data;
  std::string line;

  init_data.reserve(128);

  std::getline(in, line);

  if (!in.fail()) {
    init_data += line + "\n";
    std::getline(in, line);
  }

  if (!in.fail()) {
    init_data += line + "\n";
    std::getline(in, line);
  }

  if (!in.fail()) {
    init_data += line + "\n";
  }

  auto data = RevenuerManagerData::get(init_data);

  free_table_count = data.table_count;
  begin_time = data.begin_time;
  end_time = data.end_time;
  cost_per_hour = data.cost_per_hour;

  table_staticstic_list.resize(data.table_count);
  table_time_busy.resize(data.table_count, -1);

  prepared << formatTime(begin_time) << '\n';
}

void RevenuerManager::finalize()
{
  out << prepared.str();
  out << formatTime(end_time) << '\n';

  for (std::size_t i = 0; i < table_staticstic_list.size(); ++i) {
    out << i + 1 << ' ' << table_staticstic_list[i].revenue << ' '
        << formatTime(table_staticstic_list[i].used_time) << '\n';
  }
}

void RevenuerManager::processGeneratedEvent(const GeneratedEvent& event)
{
  switch (event.type) {
  case GeneratedEvent::Type::CLIENT_LEAVE: {
    prepared << formatTime(event.time) << " 11 " << event.client_it->first << std::endl;
    removeClient(event.time, event.client_it);
    break;
  }
  case GeneratedEvent::Type::CLIENT_TAKE_TABLE: {
    prepared << formatTime(event.time) << " 12 " << event.client_it->first << " "
             << event.table_id + 1 << std::endl;
    setClientToTable(event.time, event.client_it, event.table_id);
    break;
  }
  case GeneratedEvent::Type::ERROR: {
    prepared << formatTime(event.time) << " 13 " << event.error_message << std::endl;
    break;
  }
  }
}

void RevenuerManager::processInputEvent(const InputEvent& event)
{
  if (event.time == end_time && event.type == InputEvent::Type::CLIENT_LEAVE) {
  } else if (event.time >= end_time && !client2table.empty()) {
    deferred_event = event;

    kickOutLeftClients();

    return;
  }

  if (event.time < last_time_event) {
    throw error<std::runtime_error>("Invalid event order");
  }
  last_time_event = event.time;

  switch (event.type) {
  case InputEvent::Type::CLIENT_ARRIVE: {
    processClientArrive(event);
    break;
  }
  case InputEvent::Type::CLIENT_TAKE_TABLE: {
    processClientTakeTable(event);
    break;
  }
  case InputEvent::Type::CLIENT_WAIT: {
    processClientWait(event);
    break;
  }
  case InputEvent::Type::CLIENT_LEAVE: {
    processClientLeave(event);
    break;
  }
  default: {
    throw std::runtime_error("Unknown event type");
  }
  }
}

void RevenuerManager::processClientArrive(const InputEvent& event)
{
  if (event.time < begin_time || event.time >= end_time) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "NotOpenYet"
    });
    return;
  }

  auto it = client2table.find(event.client_id);

  if (it != client2table.end()) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "YouShallNotPass"
    });
    return;
  }

  client2table[event.client_id] = -1;
}

void RevenuerManager::processClientTakeTable(const InputEvent& event)
{
  auto it = client2table.find(event.client_id);

  if (it == client2table.end()) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "ClientUnknown"
    });
    return;
  }

  if (event.table_id >= table_time_busy.size()) {
    throw error<std::range_error>("The client attempted to sit on a non-existent table");
  }

  if (table_time_busy[event.table_id] != -1) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "PlaceIsBusy"
    });
    return;
  }

  setClientToTable(event.time, it, event.table_id);
}

void RevenuerManager::processClientWait(const InputEvent& event)
{
  auto it = client2table.find(event.client_id);

  if (it == client2table.end()) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "ClientUnknown"
    });
    return;
  }

  if (free_table_count > 0) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "ICanWaitNoLonger!"
    });
    return;
  }

  if (client_queue.size() == table_time_busy.size()) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time, .type = GeneratedEvent::Type::CLIENT_LEAVE, .client_it = it
    });
    return;
  }

  client_queue.push(std::move(event.client_id));
}

void RevenuerManager::processClientLeave(const InputEvent& event)
{
  auto it = client2table.find(event.client_id);

  if (it == client2table.end()) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::ERROR,
        .error_message = "ClientUnknown"
    });
    return;
  }

  if (it->second != -1 && !client_queue.empty()) {
    generated_event_queue.push(GeneratedEvent{
        .time = event.time,
        .type = GeneratedEvent::Type::CLIENT_TAKE_TABLE,
        .client_it = client2table.find(client_queue.front()),
        .table_id = it->second
    });
    client_queue.pop();
  }

  removeClient(event.time, it);
}

void RevenuerManager::setClientToTable(
    int current_time, std::map<ClientID, TableID>::iterator it, uint table_id
)
{
  if (it->second != -1) {
    unsetClientFromTable(current_time, it);
  }
  table_time_busy[table_id] = current_time;
  it->second = table_id;

  --free_table_count;
}

void RevenuerManager::unsetClientFromTable(
    int current_time, std::map<ClientID, TableID>::iterator it
)
{
  if (it->second == -1) {
    return;
  }
  auto passed_time = current_time - table_time_busy[it->second];
  auto hours_passed = (passed_time / 60) + ((passed_time % 60 == 0) ? 0 : 1);

  table_staticstic_list[it->second].revenue += hours_passed * cost_per_hour;
  table_staticstic_list[it->second].used_time += passed_time;

  ++free_table_count;

  table_time_busy[it->second] = -1;
}

void RevenuerManager::removeClient(
    int current_time, std::map<ClientID, TableID>::iterator it
)
{
  unsetClientFromTable(current_time, it);

  client2table.erase(it);
}

void RevenuerManager::kickOutLeftClients()
{
  for (auto it = client2table.begin(); it != client2table.end(); ++it) {
    generated_event_queue.push(GeneratedEvent{
        .time = end_time, .type = GeneratedEvent::Type::CLIENT_LEAVE, .client_it = it
    });
  }
}

} // namespace task
