#include "map_renderer.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>

namespace TransporCatalogueLib {

namespace map_render {

struct BasicInfo {

  BasicInfo(const std::deque<Domain::Bus> &buses_,
            const std::deque<Domain::Stop> &stops_) {
    buses = buses_;
    stops = stops_;
    for (auto bus : buses) {
      std::vector<Geo::Coordinates> to_save;
      if (!bus.stops_for_bus_.empty()) {
        for (auto stop : bus.stops_for_bus_) {
          raw_cords.push_back(stop->coordinates);
          to_save.push_back(stop->coordinates);
        }
        geo_by_rout.push_back(std::move(to_save));
      }
    }
  }

  std::deque<Domain::Bus> buses;
  std::deque<Domain::Stop> stops;
  std::vector<Geo::Coordinates> raw_cords;
  std::vector<std::vector<Geo::Coordinates>> geo_by_rout;
};

static void AddRoutsLines(BasicInfo &basic_info, svg::Document &to_draw,
                          RenderSettings &settings) {
  BasicInfo info = basic_info;
  std::vector<svg::Polyline> lines;
  int color_id = 0;
  const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(),
                             settings.width, settings.height, settings.padding);
  for (auto route : info.geo_by_rout) {
    svg::Polyline line_to_add;
    line_to_add.SetFillColor(svg::NoneColor)
        .SetStrokeColor(settings.color_palette[color_id])
        .SetStrokeWidth(settings.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for (auto cord : route) {
      const svg::Point screen_cord = proj(cord);
      line_to_add.AddPoint(screen_cord);
    }
    if (settings.color_palette.size() == color_id + 1) {
      color_id = 0;
    } else {
      ++color_id;
    }
    lines.push_back(line_to_add);
  }
  for (auto line : lines) {
    to_draw.Add(line);
  }
}

void AddRoutsNames(BasicInfo &basic_info, svg::Document &to_draw,
                   RenderSettings &settings) {
  BasicInfo info = basic_info;
  int color_id = 0;
  const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(),
                             settings.width, settings.height, settings.padding);
  for (const auto &bus : info.buses) {
    if (bus.stops_for_bus_.empty()) {
      continue;
    }
    if (bus.is_roundtrip == true) {
      auto stop = bus.stops_for_bus_.front()->coordinates;
      svg::Text back_text;
      back_text.SetData(bus.bus)
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
      front_text.SetData(bus.bus)
          .SetPosition(proj(stop))
          .SetOffset(settings.bus_label_offset)
          .SetFontSize(settings.bus_label_font_size)
          .SetFontFamily("Verdana")
          .SetFontWeight("bold")
          .SetFillColor(settings.color_palette[color_id]);
      to_draw.Add(front_text);
      if (settings.color_palette.size() == color_id + 1) {
        color_id = 0;
      } else {
        ++color_id;
      }
    } else {
      {
        auto stop = bus.stops_for_bus_[0]->coordinates;
        svg::Text back_text;
        back_text.SetData(bus.bus)
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
        front_text.SetData(bus.bus)
            .SetPosition(proj(stop))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(settings.color_palette[color_id]);
        to_draw.Add(front_text);
      }
      {
        auto &stops = bus.stops_for_bus_;
        auto stop = stops[stops.size() / 2];
        if (bus.stops_for_bus_.front() == stop) {
          if (settings.color_palette.size() == color_id + 1) {
            color_id = 0;
          } else {
            ++color_id;
          }
          continue;
        }
        svg::Text back_text;
        back_text.SetData(bus.bus)
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
        front_text.SetData(bus.bus)
            .SetPosition(proj(stop->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(settings.color_palette[color_id]);
        to_draw.Add(front_text);
      }
      if (settings.color_palette.size() == color_id + 1) {
        color_id = 0;
      } else {
        ++color_id;
      }
    }
  }
}

void AddStopPoints(BasicInfo &basic_info, svg::Document &to_draw,
                   RenderSettings &settings) {
  BasicInfo info = basic_info;
  const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(),
                             settings.width, settings.height, settings.padding);
  for (const auto &stop : info.stops) {
    if (stop.routs.empty()) {
      continue;
    }
    svg::Circle bus_stop;
    Geo::Coordinates stop_cords = stop.coordinates;
    bus_stop.SetCenter(proj(stop_cords))
        .SetRadius(settings.stop_radius)
        .SetFillColor("white");
    to_draw.Add(bus_stop);
  }
}

void AddStopNames(BasicInfo &basic_info, svg::Document &to_draw,
                  RenderSettings &settings) {
  BasicInfo info = basic_info;
  const SphereProjector proj(info.raw_cords.begin(), info.raw_cords.end(),
                             settings.width, settings.height, settings.padding);
  for (const auto &stop : info.stops) {
    if (stop.routs.empty()) {
      continue;
    }
    Geo::Coordinates stop_cords = stop.coordinates;
    svg::Text back_text;
    back_text.SetPosition(proj(stop_cords))
        .SetOffset(settings.stop_label_offset)
        .SetFontSize(settings.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetData(stop.stop_name)
        .SetFillColor(settings.underlayer_color)
        .SetStrokeColor(settings.underlayer_color)
        .SetStrokeWidth(settings.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    to_draw.Add(back_text);
    svg::Text front_text;
    front_text.SetData(stop.stop_name)
        .SetPosition(proj(stop_cords))
        .SetOffset(settings.stop_label_offset)
        .SetFontSize(settings.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor("black");
    to_draw.Add(front_text);
  }
}

std::ostringstream MapRender::Draw(const std::deque<Domain::Bus> &buses,
                                   const std::deque<Domain::Stop> &stops) {
  svg::Document to_draw;
  BasicInfo basic_info(buses, stops);
  AddRoutsLines(basic_info, to_draw, settings_);
  AddRoutsNames(basic_info, to_draw, settings_);
  AddStopPoints(basic_info, to_draw, settings_);
  AddStopNames(basic_info, to_draw, settings_);
  std::ostringstream to_ret;
  to_draw.Render(to_ret);
  return to_ret;
}

} // namespace map_render

} // namespace TransporCatalogueLib
