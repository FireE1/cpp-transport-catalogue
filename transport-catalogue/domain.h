#pragma once

#include "geo.h"
#include "svg.h"

#include <string>
#include <vector>

namespace Domain
{

// Структура автобуса(маршрута)
struct Bus;

// Структура остановки
struct Stop {
    // Название остановки
    std::string stop_name;
    // Гео данные(долгота, широта)
    Geo::Coordinates coordinates;
    // Отсортированное хранилище маршрутов(автобусов), проходящих через остановку
    std::vector<Bus*>routs;
};

// Структура автобуса(маршрута)
struct Bus {
    // Название пвтобуса(маршрута)
    std::string bus;
    // Хранилище ссылок на остановки, находящиеся на маршруте(автобусе)
    std::vector<const Stop*> stops_for_bus_;
};

struct RenderSettings
{
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    std::vector<svg::Color> color_palette;
};

}