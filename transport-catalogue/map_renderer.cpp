#include "map_renderer.h"

#include <iostream>
#include <algorithm>
#include <set>
#include <sstream>

namespace svg
{

namespace map_render
{

struct BasicInfo {

    BasicInfo(Handler::InfoKeeper& keeper, const TransporCatalogueLib::CatalogueCore::TransporCatalogue& cat) {
        buses = keeper.GetAllBuses();
        stops = keeper.GetAllStops();
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

static void AddRoutsLines(BasicInfo& basic_info, Document& to_draw, RenderSettings& settings) {
    BasicInfo info = basic_info;
    std::vector<Polyline> lines;
    int color_id = 0;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto route : info.geo_by_rout)
    {
        Polyline line_to_add;
        line_to_add.SetFillColor(NoneColor)
                   .SetStrokeColor(settings.color_palette[color_id])
                   .SetStrokeWidth(settings.line_width)
                   .SetStrokeLineCap(StrokeLineCap::ROUND)
                   .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        for (auto cord : route)
        {
            const Point screen_cord = proj(cord);
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

void AddRoutsNames(BasicInfo& basic_info, Document& to_draw, RenderSettings& settings, const TransporCatalogueLib::CatalogueCore::TransporCatalogue& cat) {
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
            Text back_text;
            back_text.SetData(bus_name)
                     .SetPosition(proj(stop))
                     .SetOffset(settings.bus_label_offset)
                     .SetFontSize(settings.bus_label_font_size)
                     .SetFontFamily("Verdana")
                     .SetFontWeight("bold")
                     .SetFillColor(settings.underlayer_color)
                     .SetStrokeColor(settings.underlayer_color)
                     .SetStrokeWidth(settings.underlayer_width)
                     .SetStrokeLineCap(StrokeLineCap::ROUND)
                     .SetStrokeLineJoin(StrokeLineJoin::ROUND);
            to_draw.Add(back_text);
            Text front_text;
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
                Text back_text;
                back_text.SetData(bus_name)
                         .SetPosition(proj(stop))
                         .SetOffset(settings.bus_label_offset)
                         .SetFontSize(settings.bus_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetFontWeight("bold")
                         .SetFillColor(settings.underlayer_color)
                         .SetStrokeColor(settings.underlayer_color)
                         .SetStrokeWidth(settings.underlayer_width)
                         .SetStrokeLineCap(StrokeLineCap::ROUND)
                         .SetStrokeLineJoin(StrokeLineJoin::ROUND);
                to_draw.Add(back_text);
                Text front_text;
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
                Text back_text;
                back_text.SetData(bus_name)
                         .SetPosition(proj(stop->coordinates))
                         .SetOffset(settings.bus_label_offset)
                         .SetFontSize(settings.bus_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetFontWeight("bold")
                         .SetFillColor(settings.underlayer_color)
                         .SetStrokeColor(settings.underlayer_color)
                         .SetStrokeWidth(settings.underlayer_width)
                         .SetStrokeLineCap(StrokeLineCap::ROUND)
                         .SetStrokeLineJoin(StrokeLineJoin::ROUND);
                to_draw.Add(back_text);
                Text front_text;
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

void AddStopPoints(BasicInfo& basic_info, Document& to_draw, RenderSettings& settings, const TransporCatalogueLib::CatalogueCore::TransporCatalogue& cat) {
    BasicInfo info = basic_info;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto stop_name : info.stops)
    {
        if (cat.FindStop(stop_name)->routs.empty())
        {
            continue;
        }
        Circle bus_stop;
        Geo::Coordinates stop_cords = cat.FindStop(stop_name)->coordinates;
        bus_stop.SetCenter(proj(stop_cords))
                .SetRadius(settings.stop_radius)
                .SetFillColor("white");
        to_draw.Add(bus_stop);
    }

}

void AddStopNames(BasicInfo& basic_info, Document& to_draw, RenderSettings& settings, const TransporCatalogueLib::CatalogueCore::TransporCatalogue& cat) {
    BasicInfo info = basic_info;
    const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(), settings.width, settings.height, settings.padding);
    for (auto stop_name : info.stops)
    {
        if (cat.FindStop(stop_name)->routs.empty())
        {
            continue;
        }
        Geo::Coordinates stop_cords = cat.FindStop(stop_name)->coordinates;
        Text back_text;
        back_text.SetPosition(proj(stop_cords))
                 .SetOffset(settings.stop_label_offset)
                 .SetFontSize(settings.stop_label_font_size)
                 .SetFontFamily("Verdana")
                 .SetData(stop_name)
                 .SetFillColor(settings.underlayer_color)
                 .SetStrokeColor(settings.underlayer_color)
                 .SetStrokeWidth(settings.underlayer_width)
                 .SetStrokeLineCap(StrokeLineCap::ROUND)
                 .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        to_draw.Add(back_text);
        Text front_text;
        front_text.SetData(stop_name)
                  .SetPosition(proj(stop_cords))
                  .SetOffset(settings.stop_label_offset)
                  .SetFontSize(settings.stop_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetFillColor("black");
        to_draw.Add(front_text);
    }
}

std::ostringstream MapRender::Draw(const TransporCatalogueLib::CatalogueCore::TransporCatalogue& cat, Handler::InfoKeeper& keeper) {
    Document to_draw;
    BasicInfo basic_info(keeper, cat);
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