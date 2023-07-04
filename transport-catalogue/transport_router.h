#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"

#include <unordered_map>
#include <set>
#include <string>
#include <utility>
#include <optional>

namespace TransporCatalogueLib
{

namespace TransportRouter
{

constexpr size_t DISTANCE_ = 1000;
constexpr size_t TIME_ = 60;

class Router {
public:

    void SetBusSettings(Domain::BusW_TimeAndVelocity settings);

    void SetRouter(std::vector<const Domain::Stop*>& stops, std::vector<std::pair<const Domain::Bus*, bool>>& buses, const CatalogueCore::TransporCatalogue& cat);

    std::optional<Domain::RouteInfo> BuildAndGetRoute(graph::VertexId begin, graph::VertexId end) const;

    std::optional<Domain::RouterStopWait> GetRouterOnStop(const Domain::Stop* stop) const;

private:

    Domain::BusW_TimeAndVelocity bus_settings_;
    std::unordered_map<const Domain::Stop*, Domain::RouterStopWait> stops_wait_;
    std::unordered_map<graph::EdgeId, std::variant<Domain::StopEdge, Domain::BusEdge>> fast_edge_get_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;

};

}

}