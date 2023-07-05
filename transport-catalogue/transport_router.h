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

struct BusW_TimeAndVelocity {
    double wait_time;
    double velocity;
};

struct StopEdge
{
    std::string stop_name;
    double time;
};

struct BusEdge
{
    std::string bus_name;
    size_t span;
    double time;
};

struct RouteInfo
{
    double time;
    std::vector<std::variant<StopEdge, BusEdge>> route_edges;
};

struct RouterStopWait
{
    graph::VertexId begin;
    graph::VertexId end;
};

class Router {
public:

    void SetBusSettings(BusW_TimeAndVelocity settings);

    void SetRouter(const std::vector<const Domain::Stop*>& stops, const std::vector<std::pair<const Domain::Bus*, bool>>& buses, const CatalogueCore::TransporCatalogue& cat);

    std::optional<RouteInfo> BuildAndGetRoute(const Domain::Stop* first_stop, const Domain::Stop* second_stop) const;

private:

    BusW_TimeAndVelocity bus_settings_;
    std::unordered_map<const Domain::Stop*, RouterStopWait> stops_wait_;
    std::unordered_map<graph::EdgeId, std::variant<StopEdge, BusEdge>> fast_edge_get_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;

};

}

}