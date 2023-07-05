#include "transport_router.h"

namespace TransporCatalogueLib
{

namespace TransportRouter
{

    void Router::SetBusSettings(BusW_TimeAndVelocity settings) {
        bus_settings_ = std::move(settings);
    }

    void StopEdgeAdder(const std::vector<const Domain::Stop*>& stops,
                        std::unique_ptr<graph::DirectedWeightedGraph<double>>& graph,
                        std::unordered_map<const Domain::Stop*, RouterStopWait>& stops_wait,
                        BusW_TimeAndVelocity& bus_settings,
                        std::unordered_map<graph::EdgeId, std::variant<StopEdge, BusEdge>>& fast_edge_get) {
        size_t rt = 0;
        for (auto stop : stops)
        {
            graph::VertexId begin = rt++;
            graph::VertexId end = rt++;
            stops_wait[stop] = RouterStopWait{begin, end};
        }
        for (auto [stop, wait] : stops_wait)
        {
            graph::EdgeId id = graph->AddEdge(graph::Edge<double>{wait.begin, wait.end, bus_settings.wait_time});
            fast_edge_get[id] = StopEdge{stop->stop_name, bus_settings.wait_time};
        }
    }
    
    void AddBusEdge (const Domain::Bus* bus, const CatalogueCore::TransporCatalogue& cat,
                        std::unique_ptr<graph::DirectedWeightedGraph<double>>& graph,
                        std::unordered_map<const Domain::Stop*, RouterStopWait>& stops_wait,
                        BusW_TimeAndVelocity& bus_settings,
                        std::unordered_map<graph::EdgeId, std::variant<StopEdge, BusEdge>>& fast_edge_get) {
        for (auto stop = bus->stops_for_bus_.begin(); stop != bus->stops_for_bus_.end(); ++stop)
        {
            size_t distance = 0;
            size_t span = 0;
            for (auto next_stop = std::next(stop); next_stop != bus->stops_for_bus_.end(); ++next_stop)
            {
                distance += cat.GetDistanceBetweenStops(*std::prev(next_stop), *next_stop);
                ++span;
                graph::EdgeId id = graph->AddEdge(graph::Edge<double>{stops_wait.at(*stop).end, stops_wait.at(*next_stop).begin, distance * 1.0 / (bus_settings.velocity * DISTANCE_ / TIME_)});
                fast_edge_get[id] = BusEdge{bus->bus, span, graph->GetEdge(id).weight};
            }
        }
    }

    void BusEdgeAdder(const std::vector<std::pair<const Domain::Bus*, bool>>& buses,
                        const CatalogueCore::TransporCatalogue& cat,
                        std::unique_ptr<graph::DirectedWeightedGraph<double>>& graph,
                        std::unordered_map<const Domain::Stop*, RouterStopWait>& stops_wait,
                        BusW_TimeAndVelocity& bus_settings,
                        std::unordered_map<graph::EdgeId, std::variant<StopEdge, BusEdge>>& fast_edge_get) {
        for (auto [bus, round] : buses)
        {
            AddBusEdge(bus, cat, graph, stops_wait, bus_settings, fast_edge_get);
            if (!round)
            {
                AddBusEdge(bus, cat, graph, stops_wait, bus_settings, fast_edge_get);
            }
        }
    }

    void Router::SetRouter(const std::vector<const Domain::Stop*>& stops, const std::vector<std::pair<const Domain::Bus*, bool>>& buses, const CatalogueCore::TransporCatalogue& cat) {
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(stops.size() * 2);
        StopEdgeAdder(stops, graph_, stops_wait_, bus_settings_, fast_edge_get_);
        BusEdgeAdder(buses, cat, graph_, stops_wait_, bus_settings_, fast_edge_get_);
        router_ = std::make_unique<graph::Router<double>>(*graph_);
        router_->Build();
    }

    std::optional<RouterStopWait> GetRouterOnStop(const Domain::Stop* stop, const std::unordered_map<const Domain::Stop*, RouterStopWait>& stops_wait) {
        if (stops_wait.count(stop))
        {
            return stops_wait.at(stop);
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<RouteInfo> Router::BuildAndGetRoute(const Domain::Stop* first_stop, const Domain::Stop* second_stop) const {
        const std::optional<graph::Router<double>::RouteInfo>& route = router_->BuildRoute(GetRouterOnStop(first_stop, stops_wait_)->begin, GetRouterOnStop(second_stop, stops_wait_)->begin);
        if (route.has_value())
        {
            RouteInfo to_ret;
            to_ret.time = route->weight;
            for (const auto edge : route->edges)
            {
                to_ret.route_edges.emplace_back(fast_edge_get_.at(edge));
            }
            return to_ret;
        }
        return std::nullopt;
    }

}

}