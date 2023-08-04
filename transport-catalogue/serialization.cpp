#include "serialization.h"

#include <map>

namespace TransporCatalogueLib
{

namespace Serialization
{

void SerializationClass::SerializeCatalogue(const CatalogueCore::TransporCatalogue& cat) {
    proto_transport_catalogue::TransporCatalogueProto serialized_cat;
    std::map<std::string, int> stop_to_id;
    {
        int stop_id = 0;
        std::set<std::string> sorted_stops;
        for (const auto& stop : cat.GetStops())
        {
            sorted_stops.insert(stop.stop_name);
        }
        for (const auto& stop_to_add : sorted_stops)
        {
            proto_transport_catalogue::Stop serialized_stop;
            serialized_stop.set_id(stop_id);
            serialized_stop.set_stop_name(stop_to_add);
            stop_to_id.emplace(stop_to_add, stop_id);
            serialized_stop.set_lat(cat.FindStop(stop_to_add)->coordinates.lat);
            serialized_stop.set_lng(cat.FindStop(stop_to_add)->coordinates.lng);
            *serialized_cat.add_stops() = std::move(serialized_stop);
            ++stop_id;
        }
    }
    for (const auto& [from, to_dist] : cat.GetDistances())
    {
        for (const auto& [to, dist] : to_dist)
        {
            proto_transport_catalogue::StopToStopDist serialized_distance;
            serialized_distance.set_stop_from(stop_to_id.at(std::string(from)));
            serialized_distance.set_stop_to(stop_to_id.at(std::string(to)));
            serialized_distance.set_distance(dist);
            *serialized_cat.add_from_stop_to_stop() = std::move(serialized_distance);
        }
    }
    {
        std::set<std::string> sorted_buses;
        for (const auto& bus : cat.GetBuses())
        {
            sorted_buses.insert(bus.bus);
        }
        for (const auto& bus_to_add : sorted_buses)
        {
            proto_transport_catalogue::Bus serialized_bus;
            serialized_bus.set_bus_name(bus_to_add);
            for (const auto& bus_stop : cat.FindBus(bus_to_add)->stops_for_bus_)
            {
                serialized_bus.add_stops(stop_to_id.at(bus_stop->stop_name));
            }
            serialized_bus.set_is_roundtrit(cat.FindBus(bus_to_add)->is_roundtrip);
            *serialized_cat.add_buses() = std::move(serialized_bus);
        }
    }
    *serialized_info_.mutable_catalogue() = std::move(serialized_cat);
}

proto_transport_svg::Color SerializeColor(const svg::Color& color) {
    proto_transport_svg::Color serialized_color;
    if (std::holds_alternative<std::monostate>(color))
    {
        serialized_color.set_none(true);
    }
    else if (std::holds_alternative<svg::Rgb>(color))
    {
        svg::Rgb rgb = std::get<svg::Rgb>(color);
        proto_transport_svg::Rgb serialized_rgb;
        serialized_rgb.set_red(rgb.red);
        serialized_rgb.set_green(rgb.green);
        serialized_rgb.set_blue(rgb.blue);
        *serialized_color.mutable_rgb() = serialized_rgb;
    }
    else if (std::holds_alternative<svg::Rgba>(color))
    {
        svg::Rgba rgba = std::get<svg::Rgba>(color);
        proto_transport_svg::Rgba serialized_rgba;
        serialized_rgba.set_red(rgba.red);
        serialized_rgba.set_green(rgba.green);
        serialized_rgba.set_blue(rgba.blue);
        serialized_rgba.set_opacity(rgba.alpha);
        *serialized_color.mutable_rgba() = serialized_rgba;
    }
    else if (std::holds_alternative<std::string>(color))
    {
        serialized_color.set_string(std::get<std::string>(color));
    }
    return serialized_color;
}

void SerializationClass::SerializeRenderSettings(const map_render::RenderSettings& settings) {
    proto_transport_map_render::RenderSettingsProto serialized_settings;
    serialized_settings.set_width(settings.width);
    serialized_settings.set_height(settings.height);
    serialized_settings.set_padding(settings.padding);
    serialized_settings.set_line_width(settings.line_width);
    serialized_settings.set_stop_radius(settings.stop_radius);
    serialized_settings.set_bus_label_font_size(settings.bus_label_font_size);
    {
        proto_transport_svg::Point bus_label_offset;
        bus_label_offset.set_x(settings.bus_label_offset.x);
        bus_label_offset.set_y(settings.bus_label_offset.y);
        *serialized_settings.mutable_bus_label_offset() = bus_label_offset;
    }
    serialized_settings.set_stop_label_font_size(settings.stop_label_font_size);
    {
        proto_transport_svg::Point stop_label_offset;
        stop_label_offset.set_x(settings.stop_label_offset.x);
        stop_label_offset.set_y(settings.stop_label_offset.y);
        *serialized_settings.mutable_stop_label_offset() = stop_label_offset;
    }
    *serialized_settings.mutable_underlayer_color() = SerializeColor(settings.underlayer_color);
    serialized_settings.set_underlayer_width(settings.underlayer_width);
    for (const auto& pallete : settings.color_palette)
    {
        *serialized_settings.add_color_palette() = SerializeColor(pallete);
    }
    *serialized_info_.mutable_map_render_settings() = std::move(serialized_settings);
}

void SerializationClass::SerializeRouting(const TransportRouter::Router& routing) {
    proto_transport_router::RouterProto serialized_routing;
    {
        const auto& bus_settings = routing.BusSettingsGetter();
        proto_transport_router::BusW_TimeAndVelocity serialized_settings;
        serialized_settings.set_wait_time(bus_settings.wait_time);
        serialized_settings.set_velocity(bus_settings.velocity);
        *serialized_routing.mutable_settings() = serialized_settings;
    }
    for (const auto& [stop, rsw] : routing.StopsWaitGetter())
    {
        proto_transport_router::StopWait serialized_stop_wait;
        serialized_stop_wait.set_stop(stop->stop_name);
        proto_transport_router::RouterStopWait serialized_rsw;
        serialized_rsw.set_begin(rsw.begin);
        serialized_rsw.set_end(rsw.end);
        *serialized_stop_wait.mutable_stop_wait() = serialized_rsw;
        *serialized_routing.add_stop_wait() = serialized_stop_wait;
    }
    
    for (const auto& [edge, edge_type] : routing.FastEdgeGetGetter())
    {
        proto_transport_router::EgdeGet serialized_edge_get;
        serialized_edge_get.set_edge_id(edge);
        proto_transport_router::RouteEdges serialized_edge_type;
        if (std::holds_alternative<TransportRouter::StopEdge>(edge_type))
        {
            TransportRouter::StopEdge stop_edge = std::get<TransportRouter::StopEdge>(edge_type);
            proto_transport_router::StopEdge serialized_stop_edge;
            serialized_stop_edge.set_stop_name(stop_edge.stop_name);
            serialized_stop_edge.set_time(stop_edge.time);
            *serialized_edge_type.mutable_stop_edge() = serialized_stop_edge;
        }
        else if (std::holds_alternative<TransportRouter::BusEdge>(edge_type))
        {
            TransportRouter::BusEdge bus_edge = std::get<TransportRouter::BusEdge>(edge_type);
            proto_transport_router::BusEdge serialized_bus_edge;
            serialized_bus_edge.set_bus_name(bus_edge.bus_name);
            serialized_bus_edge.set_span(bus_edge.span);
            serialized_bus_edge.set_time(bus_edge.time);
            *serialized_edge_type.mutable_bus_edge() = serialized_bus_edge;
        }
        *serialized_edge_get.mutable_route_edges() = serialized_edge_type;
        *serialized_routing.add_edge_get() = serialized_edge_get;
    }

    {
        const auto& graph_edges = routing.GraphGetter()->GetEdges();
        proto_transport_graph::GraphProto serialized_graph;
        for (const auto& edge : graph_edges)
        {
            proto_transport_graph::Edge serialized_edge;
            serialized_edge.set_from(edge.from);
            serialized_edge.set_to(edge.to);
            serialized_edge.set_weight(edge.weight);
            *serialized_graph.add_edges_() = serialized_edge;
        }
        const auto& graph_incidence_lists = routing.GraphGetter()->GetIncidenceLists();
        for (const auto& incidence_list : graph_incidence_lists)
        {
            proto_transport_graph::IncidenceList serialized_lists;
            for (const auto& list : incidence_list)
            {
                serialized_lists.add_incidence_list(list);
            }
            *serialized_graph.add_incidence_lists() = serialized_lists;
        }
        *serialized_routing.mutable_graph() = serialized_graph;
    }
    {
        const auto& router_internal = *(routing.RouterGetter()->GetInternal());
        proto_transport_graph::RouterProto serialized_router;
        proto_transport_graph::RoutesInternalData serialized_internals;
        for (const auto& rts_vect : router_internal)
        {
            proto_transport_graph::RouteInternalData_vector serialized_rts_vect;
            for (const auto& internal : rts_vect)
            {
                proto_transport_graph::RouteInternalData serialized_internal;
                if (internal.has_value())
                {
                    serialized_internal.set_nullopt(false);
                    serialized_internal.set_weight(internal.value().weight);
                    if (internal.value().prev_edge.has_value())
                    {
                        serialized_internal.set_prev_edge_set(true);
                        serialized_internal.set_prev_edge(internal.value().prev_edge.value());
                    }
                    else
                    {
                        serialized_internal.set_prev_edge_set(false);
                    }
                }
                else
                {
                    serialized_internal.set_nullopt(true);
                }
                *serialized_rts_vect.add_route_internal_data_vector() = serialized_internal;
            }
            *serialized_internals.add_routes_internal_data() = serialized_rts_vect;
        }
        *serialized_router.mutable_routes_internal_data() = serialized_internals;
        *serialized_routing.mutable_router() = serialized_router;
    }
    *serialized_info_.mutable_router() = std::move(serialized_routing);
}

CatalogueCore::TransporCatalogue SerializationClass::DeserializeCatalogue() {
    CatalogueCore::TransporCatalogue cat;
    const auto& serialized_cat = serialized_info_.catalogue();
    const auto& serialized_stops = serialized_cat.stops();
    for (const auto& serialized_stop : serialized_stops)
    {
        Domain::Stop stop;
        stop.stop_name = serialized_stop.stop_name();
        stop.coordinates = {serialized_stop.lat(), serialized_stop.lng()};
        cat.AddStop(std::move(stop));
    }
    {
        const auto& serialized_distances = serialized_cat.from_stop_to_stop();
        for (const auto& serialized_distance : serialized_distances)
        {
            std::string from = serialized_stops[serialized_distance.stop_from()].stop_name();
            std::string to = serialized_stops[serialized_distance.stop_to()].stop_name();
            size_t distance = serialized_distance.distance();
            cat.SetStopToStopDistance(from, to, distance);
        }
    }
    {
        const auto& serialized_buses = serialized_cat.buses();
        for (const auto& serialized_bus : serialized_buses)
        {
            Domain::Bus bus;
            bus.bus = serialized_bus.bus_name();
            for (const auto& bus_stop : serialized_bus.stops())
            {
                bus.stops_for_bus_.push_back(cat.FindStop(serialized_stops[bus_stop].stop_name()));
            }
            bus.is_roundtrip = serialized_bus.is_roundtrit();
            cat.AddBus(std::move(bus));
        }
    }
    return cat;
}

svg::Color DeserializeColor(const proto_transport_svg::Color& serialized_color) {
    svg::Color color;
    if (serialized_color.has_rgb())
    {
        const auto& serialized_rgb = serialized_color.rgb();
        svg::Rgb rgb;
        rgb.red = serialized_rgb.red();
        rgb.green = serialized_rgb.green();
        rgb.blue = serialized_rgb.blue();
        color = rgb;
    }
    else if (serialized_color.has_rgba())
    {
        const auto& serialized_rgba = serialized_color.rgba();
        svg::Rgba rgba;
        rgba.red = serialized_rgba.red();
        rgba.green = serialized_rgba.green();
        rgba.blue = serialized_rgba.blue();
        rgba.alpha = serialized_rgba.opacity();
        color = rgba;
    }
    else
    {
        color = serialized_color.string();
    }
    return color;
}

map_render::RenderSettings SerializationClass::DeserializeRenderSettings() {
    map_render::RenderSettings settings;
    const auto& serialized_settings = serialized_info_.map_render_settings();
    settings.width = serialized_settings.width();
    settings.height = serialized_settings.height();
    settings.padding = serialized_settings.padding();
    settings.line_width = serialized_settings.line_width();
    settings.stop_radius = serialized_settings.stop_radius();
    settings.bus_label_font_size = serialized_settings.bus_label_font_size();
    {
        const auto& serialized_bus_label_offset = serialized_settings.bus_label_offset();
        svg::Point bus_label_offset;
        bus_label_offset.x = serialized_bus_label_offset.x();
        bus_label_offset.y = serialized_bus_label_offset.y();
        settings.bus_label_offset = bus_label_offset;
    }
    settings.stop_label_font_size = serialized_settings.stop_label_font_size();
    {
        const auto& serialized_stop_label_offset = serialized_settings.stop_label_offset();
        svg::Point stop_label_offset;
        stop_label_offset.x = serialized_stop_label_offset.x();
        stop_label_offset.y = serialized_stop_label_offset.y();
        settings.stop_label_offset = stop_label_offset;
    }
    settings.underlayer_color = DeserializeColor(serialized_settings.underlayer_color());
    settings.underlayer_width = serialized_settings.underlayer_width();
    for (const auto& pallete : serialized_settings.color_palette())
    {
        settings.color_palette.push_back(DeserializeColor(pallete));
    }
    return settings;
}

TransportRouter::Router SerializationClass::DeserializeRouting(const CatalogueCore::TransporCatalogue& cat) {
    // получаем настройки автобуса
    //const auto& serialized_settings = serialized_routing.settings();
    //TransportRouter::BusW_TimeAndVelocity routing_settings = {serialized_settings.wait_time(), serialized_settings.velocity()};
    const auto& serialized_routing = serialized_info_.router();
    TransportRouter::BusW_TimeAndVelocity routing_settings = {serialized_routing.settings().wait_time(), serialized_routing.settings().velocity()};
    // инициализируем роутер и вкидываем в него настройки
    TransportRouter::Router routing;
    routing.SetBusSettings(routing_settings);
    // вкидываем stop_wait
    //const auto& serialized_stop_wait = serialized_routing.stop_wait();
    {
        std::vector<std::pair<const Domain::Stop*, TransportRouter::RouterStopWait>> to_insert_sw;
        //for (const auto& stop_wait : serialized_stop_wait)
        for (const auto& stop_wait : serialized_routing.stop_wait())
        {
            proto_transport_router::RouterStopWait serialized_rsw = stop_wait.stop_wait();
            TransportRouter::RouterStopWait begin_end = {serialized_rsw.begin(), serialized_rsw.end()};
            to_insert_sw.push_back(std::make_pair(cat.FindStop(stop_wait.stop()), begin_end));
        }
        routing.MutableStopsWaitGetter().insert(to_insert_sw.rbegin(), to_insert_sw.rend());
    }
    // вкидываем быстрое получение edge
    //const auto& serialized_edge_get = serialized_routing.edge_get();
    {
        std::vector<std::pair<graph::EdgeId, std::variant<TransportRouter::StopEdge, TransportRouter::BusEdge>>> to_insert_feg;
        //for (const auto& edge_get : serialized_edge_get)
        for (const auto& edge_get : serialized_routing.edge_get())
        {
            const auto& edge_id = edge_get.edge_id();
            proto_transport_router::RouteEdges type = edge_get.route_edges();
            if (type.has_stop_edge())
            {
                std::string stop_name = type.stop_edge().stop_name();
                double time = type.stop_edge().time();
                TransportRouter::StopEdge edge = {stop_name, time};
                to_insert_feg.push_back(std::make_pair(edge_id, edge));
            }
            else if (type.has_bus_edge())
            {
                std::string bus_name = type.bus_edge().bus_name();
                size_t span = type.bus_edge().span();
                double time = type.bus_edge().time();
                TransportRouter::BusEdge edge = {bus_name, span, time};
                to_insert_feg.push_back(std::make_pair(edge_id, edge));
            }
        }
        routing.MutableFastEdgeGetGetter().insert(to_insert_feg.rbegin(), to_insert_feg.rend());
    }
    // разбираемся с graph
    {
    auto graph = std::make_unique<graph::DirectedWeightedGraph<double>>() ;
        // graph edges
        {
            auto& graph_edges = graph->GetEdges();
            const auto& serialized_graph = serialized_routing.graph();
            for (const auto& serialized_grp_egde : serialized_graph.edges_())
            {
                size_t from = serialized_grp_egde.from();
                size_t to = serialized_grp_egde.to();
                double weight = serialized_grp_egde.weight();
                graph::Edge<double> edge = {from, to, weight};
                graph_edges.push_back(edge);
            }
        }
        // graph incidence lists
        {
            auto& graph_incidence_lists = graph->GetIncidenceLists();
            //for (const auto& serialized_grp_incidence_list : serialized_graph.incidence_lists())
            for (const auto& serialized_grp_incidence_list : serialized_routing.graph().incidence_lists())
            {
                std::vector<size_t> incodence_list;
                for (const auto& serialized_incedence : serialized_grp_incidence_list.incidence_list())
                {
                    incodence_list.push_back(serialized_incedence);
                }
                graph_incidence_lists.push_back(incodence_list);
            }
        }
        routing.MutableGraphGetter() = std::move(graph);
    }
    // разбираемся с routing
    routing.MutableRouterGetter() = std::move(std::make_unique<graph::Router<double>>(*routing.GraphGetter()));
    //const auto& serialized_router = serialized_routing.router();
    //const auto& serialized_rid = serialized_router.routes_internal_data();
    int outer_vector_id = 0;
    //for (const auto& serialized_internal_st1 : serialized_rid.routes_internal_data())
    for (const auto& serialized_internal_st1 : serialized_routing.router().routes_internal_data().routes_internal_data())
    {
        int inner_vector_id = 0;
        for (const auto& serialized_internal : serialized_internal_st1.route_internal_data_vector())
        {
            if (!serialized_internal.nullopt())
            {
                if (serialized_internal.prev_edge_set())
                {
                    double weight = serialized_internal.weight();
                    graph::Router<double>::RouteInternalData to_add;
                    to_add.weight = weight;
                    to_add.prev_edge.emplace(serialized_internal.prev_edge());
                    *routing.MutableRouterGetter()->SetInternal(outer_vector_id, inner_vector_id) = to_add;
                }
                else
                {
                    double weight = serialized_internal.weight();
                    graph::Router<double>::RouteInternalData to_add;
                    to_add.weight = weight;
                    *routing.MutableRouterGetter()->SetInternal(outer_vector_id, inner_vector_id) = to_add;
                }
            }
            ++inner_vector_id;
        }
        ++outer_vector_id;
    }
    return routing;
}

}

}