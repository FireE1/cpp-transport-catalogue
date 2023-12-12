#include "Map_renderer/map_renderer.h"
#include "Request_handler/request_handler.h"
#include "Router/transport_router.h"
#include "SVG/svg.h"
#include "Serialization/serialization.h"
#include "Transport_Catalogue/transport_catalogue.h"
#include "Json/json.h"
#include "Json/json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr) {
  stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    PrintUsage();
    return 1;
  }

  const std::string_view mode(argv[1]);

  if (mode == "make_base"sv) {

    using namespace TransporCatalogueLib;
    Serialization::SerializationClass serialization;
    std::string file_name;
    {
      Handler::InfoKeeper keeper;
      keeper.GetInput() = json::Load(std::cin);
      try {
        file_name = keeper.GetInput()
                        .GetRoot()
                        .AsMap()
                        .at("serialization_settings")
                        .AsMap()
                        .at("file")
                        .AsString();
      } catch (...) {
      }
      jsonOperator::Reader(serialization, keeper, true);
    }
    std::ofstream out(file_name, std::ios::binary);
    serialization.GetSerializedInfo()->SerializeToOstream(&out);

  } else if (mode == "process_requests"sv) {

    using namespace TransporCatalogueLib;
    Serialization::SerializationClass serialization;
    Handler::InfoKeeper keeper;
    {
      keeper.GetInput() = json::Load(std::cin);
      std::string file_name;
      try {
        file_name = keeper.GetInput()
                        .GetRoot()
                        .AsMap()
                        .at("serialization_settings")
                        .AsMap()
                        .at("file")
                        .AsString();
      } catch (...) {
      }
      std::ifstream in(file_name, std::ios::binary);
      serialization.MutableGetSerializedInfo()->ParseFromIstream(&in);
    }
    jsonOperator::Reader(serialization, keeper, false);
    json::Print(keeper.GetOutput(), std::cout);

  } else {
    PrintUsage();
    return 1;
  }
}
