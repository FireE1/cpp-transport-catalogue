#pragma once

#include "../Map_renderer/map_renderer.h"
#include "../Router/transport_router.h"
#include "../Transport_Catalogue/transport_catalogue.h"
#include "proto/transport_catalogue.pb.h"

namespace TransporCatalogueLib {

namespace Serialization {

class SerializationClass {
public:
  void SerializeCatalogue(const CatalogueCore::TransporCatalogue &cat);
  void SerializeRenderSettings(const map_render::RenderSettings &settings);
  void SerializeRouting(const TransportRouter::Router &routing);

  CatalogueCore::TransporCatalogue DeserializeCatalogue();
  map_render::RenderSettings DeserializeRenderSettings();
  TransportRouter::Router
  DeserializeRouting(const CatalogueCore::TransporCatalogue &cat);

  proto_transport_catalogue::ComplexCatalogueInfoProto *
  MutableGetSerializedInfo() {
    return &serialized_info_;
  }

  const proto_transport_catalogue::ComplexCatalogueInfoProto *
  GetSerializedInfo() const {
    return &serialized_info_;
  }

private:
  proto_transport_catalogue::ComplexCatalogueInfoProto serialized_info_;
};

} // namespace Serialization

} // namespace TransporCatalogueLib
