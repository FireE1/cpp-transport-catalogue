#include "map_renderer.h"

#include <iostream>
#include <algorithm>
#include <set>
#include <sstream>

namespace TransporCatalogueLib
{

namespace map_render
{

struct BasicInfo {

    BasicInfo(std::set<std::pair<std::string, bool>> buses_, std::set<std::string> stops_, const CatalogueCore::TransporCatalogue& cat) {
        buses = buses_;
        stops = stops_;
        for (auto [bus_name, _] : buses)
        {
            std::vector<Geo::Coordinates> to_save;
            auto bus = cat.FindBus(bus_name);
            if (!bus->stops_for_bus_.empty())
            {
                for (auto stop : bus->stops_for_bus_)
                {
                    raw_cords.push_back(stop->coordinates);
                    to_save.push_back(stop->coordinates);
                }
                geo_by_rout.push_back(std::move(to_save));
            }
        }
    }

    std::set<std::pair<std::string, bool>> buses;
    std::set<std::string> stops;
    std::vector<Geo::Coordinates> raw_cords;
    std::vector<std::vector<Geo::Coordinates>> geo_by_rout;

};

static void AddRoutsLines(BasicInfo& basic_info, svg::Document& to_draw, Domain::RenderSettings& settings) {
    BasicInfo info = basic_info;
    std::vector<svg::Polyline> lines;
    int color_id = 0;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto route : info.geo_by_rout)
    {
        svg::Polyline line_to_add;
        line_to_add.SetFillColor(svg::NoneColor)
                   .SetStrokeColor(settings.color_palette[color_id])
                   .SetStrokeWidth(settings.line_width)
                   .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                   .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for (auto cord : route)
        {
            const svg::Point screen_cord = proj(cord);
            line_to_add.AddPoint(screen_cord);    
        }
        if(settings.color_palette.size() == color_id + 1)
        {
            color_id = 0;
        }
        else
        {
            ++color_id;
        }
        lines.push_back(line_to_add);
    }
    for (auto line : lines)
    {
        to_draw.Add(line);
    }
}

void AddRoutsNames(BasicInfo& basic_info, svg::Document& to_draw, Domain::RenderSettings& settings, const CatalogueCore::TransporCatalogue& cat) {
    BasicInfo info = basic_info;
    int color_id = 0;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto [bus_name, is_round] : info.buses)
    {
        if (cat.FindBus(bus_name)->stops_for_bus_.empty())
        {
            continue;
        }
        if (is_round == true)
        {
            auto stop = cat.FindBus(bus_name)->stops_for_bus_.front()->coordinates;
            svg::Text back_text;
            back_text.SetData(bus_name)
                     .SetPosition(proj(stop))
                     .SetOffset(settings.bus_label_offset)
                     .SetFontSize(settings.bus_label_font_size)
                     .SetFontFamily("Verdana")
                     .SetFontWeight("bold")
                     .SetFillColor(settings.underlayer_color)
                     .SetStrokeColor(settings.underlayer_color)
                     .SetStrokeWidth(settings.underlayer_width)
                     .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                     .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            to_draw.Add(back_text);
            svg::Text front_text;
            front_text.SetData(bus_name)
                      .SetPosition(proj(stop))
                      .SetOffset(settings.bus_label_offset)
                      .SetFontSize(settings.bus_label_font_size)
                      .SetFontFamily("Verdana")
                      .SetFontWeight("bold")
                      .SetFillColor(settings.color_palette[color_id]);
            to_draw.Add(front_text);
            if(settings.color_palette.size() == color_id + 1)
            {
                color_id = 0;
            }
            else
            {
                ++color_id;
            }
        }
        else
        {
            {
                auto stop = cat.FindBus(bus_name)->stops_for_bus_[0]->coordinates;
                svg::Text back_text;
                back_text.SetData(bus_name)
                         .SetPosition(proj(stop))
                         .SetOffset(settings.bus_label_offset)
                         .SetFontSize(settings.bus_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetFontWeight("bold")
                         .SetFillColor(settings.underlayer_color)
                         .SetStrokeColor(settings.underlayer_color)
                         .SetStrokeWidth(settings.underlayer_width)
                         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                to_draw.Add(back_text);
                svg::Text front_text;
                front_text.SetData(bus_name)
                          .SetPosition(proj(stop))
                          .SetOffset(settings.bus_label_offset)
                          .SetFontSize(settings.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetFillColor(settings.color_palette[color_id]);
                to_draw.Add(front_text);
            }
            {
                auto& stops = cat.FindBus(bus_name)->stops_for_bus_;
                auto stop = stops[stops.size() / 2];
                if (cat.FindBus(bus_name)->stops_for_bus_.front() == stop)
                {
                    if(settings.color_palette.size() == color_id + 1)
                    {
                        color_id = 0;
                    }
                    else
                    {
                        ++color_id;
                    }
                    continue;
                }
                svg::Text back_text;
                back_text.SetData(bus_name)
                         .SetPosition(proj(stop->coordinates))
                         .SetOffset(settings.bus_label_offset)
                         .SetFontSize(settings.bus_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetFontWeight("bold")
                         .SetFillColor(settings.underlayer_color)
                         .SetStrokeColor(settings.underlayer_color)
                         .SetStrokeWidth(settings.underlayer_width)
                         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                to_draw.Add(back_text);
                svg::Text front_text;
                front_text.SetData(bus_name)
                          .SetPosition(proj(stop->coordinates))
                          .SetOffset(settings.bus_label_offset)
                          .SetFontSize(settings.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetFillColor(settings.color_palette[color_id]);
                to_draw.Add(front_text);
            }
            if(settings.color_palette.size() == color_id + 1)
            {
                color_id = 0;
            }
            else
            {
                ++color_id;
            }
        }
    }
}

void AddStopPoints(BasicInfo& basic_info, svg::Document& to_draw, Domain::RenderSettings& settings, const CatalogueCore::TransporCatalogue& cat) {
    BasicInfo info = basic_info;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto stop_name : info.stops)
    {
        if (cat.FindStop(stop_name)->routs.empty())
        {
            continue;
        }
        svg::Circle bus_stop;
        Geo::Coordinates stop_cords = cat.FindStop(stop_name)->coordinates;
        bus_stop.SetCenter(proj(stop_cords))
                .SetRadius(settings.stop_radius)
                .SetFillColor("white");
        to_draw.Add(bus_stop);
    }

}

void AddStopNames(BasicInfo& basic_info, svg::Document& to_draw, Domain::RenderSettings& settings, const CatalogueCore::TransporCatalogue& cat) {
    BasicInfo info = basic_info;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto stop_name : info.stops)
    {
        if (cat.FindStop(stop_name)->routs.empty())
        {
            continue;
        }
        Geo::Coordinates stop_cords = cat.FindStop(stop_name)->coordinates;
        svg::Text back_text;
        back_text.SetPosition(proj(stop_cords))
                 .SetOffset(settings.stop_label_offset)
                 .SetFontSize(settings.stop_label_font_size)
                 .SetFontFamily("Verdana")
                 .SetData(stop_name)
                 .SetFillColor(settings.underlayer_color)
                 .SetStrokeColor(settings.underlayer_color)
                 .SetStrokeWidth(settings.underlayer_width)
                 .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                 .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        to_draw.Add(back_text);
        svg::Text front_text;
        front_text.SetData(stop_name)
                  .SetPosition(proj(stop_cords))
                  .SetOffset(settings.stop_label_offset)
                  .SetFontSize(settings.stop_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetFillColor("black");
        to_draw.Add(front_text);
    }
}

std::ostringstream MapRender::Draw(const CatalogueCore::TransporCatalogue& cat, std::set<std::pair<std::string, bool>> buses, std::set<std::string> stops) {
    svg::Document to_draw;
    BasicInfo basic_info(buses, stops, cat);
    AddRoutsLines(basic_info, to_draw, settings_);
    AddRoutsNames(basic_info, to_draw, settings_, cat);
    AddStopPoints(basic_info, to_draw, settings_, cat);
    AddStopNames(basic_info, to_draw, settings_, cat);
    std::ostringstream to_ret;
    to_draw.Render(to_ret);
    return to_ret;
}

}

}