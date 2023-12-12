#pragma once

#include "../Json/json.h"

#include <set>
#include <string>
#include <utility>

namespace TransporCatalogueLib {

namespace Handler {

class InfoKeeper {
public:
  json::Document &GetInput() { return input; }
  json::Document &GetOutput() { return output; }
  void SaveBus(std::string &bus, bool is_round) {
    name_of_buses.insert(std::make_pair(bus, is_round));
  }
  void SaveStop(std::string stop) { name_of_stops.insert(stop); }
  std::set<std::pair<std::string, bool>> &GetAllBuses() {
    return name_of_buses;
  }
  std::set<std::string> &GetAllStops() { return name_of_stops; }

private:
  json::Document input;
  json::Document output;
  std::set<std::pair<std::string, bool>> name_of_buses;
  std::set<std::string> name_of_stops;
};

} // namespace Handler

} // namespace TransporCatalogueLib
