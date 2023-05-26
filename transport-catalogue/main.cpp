#include "transport_catalogue.h"
#include "transport_catalogue.cpp"

#include "json.h"
#include "json.cpp"
#include "json_reader.h"
#include "json_reader.cpp"
#include "svg.h"
#include "svg.cpp"
#include "map_renderer.h"
#include "map_renderer.cpp"
#include "request_handler.h"

#include <iostream>

int main() {
    {
        using namespace TransporCatalogueLib;
        CatalogueCore::TransporCatalogue cat;
        Handler::InfoKeeper keeper;
        keeper.GetInput() = json::Load(std::cin);
        svg::map_render::MapRender drawing;
        json::Reader(keeper, cat, drawing);
        json::Print(keeper.GetOutput(), std::cout);
    }
}