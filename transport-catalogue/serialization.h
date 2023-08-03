#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>

namespace TransporCatalogueLib
{

namespace Serialization
{

class SerializationClass {
public:

    void SerializeCatalogue(const CatalogueCore::TransporCatalogue& cat);
    void SerializeRenderSettings(const map_render::RenderSettings& settings);
    void SerializeRouting(const TransportRouter::Router& routing, const transport_catalogue::TransporCatalogue& serialized_cat);

    void Deserialize();

    transport_catalogue::ComplexCatalogueInfo* MutableGetSerializedInfo() {return &serialized_info_;}
    CatalogueCore::TransporCatalogue* MutableGetCatalogue() {return &cat_;}
    map_render::MapRender* MutableGetDrawing() {return &drawing_;}
    TransportRouter::Router* MutableGetRouting() {return &routing_;}

    const transport_catalogue::ComplexCatalogueInfo* GetSerializedInfo() const {return &serialized_info_;}
    const CatalogueCore::TransporCatalogue* GetCatalogue() const {return &cat_;}
    const map_render::MapRender* GetDrawing() const {return &drawing_;}
    const TransportRouter::Router* GetRouting() const {return &routing_;}

private:

    transport_catalogue::ComplexCatalogueInfo serialized_info_;
    CatalogueCore::TransporCatalogue cat_;
    map_render::MapRender drawing_;
    TransportRouter::Router routing_;

};

}

}