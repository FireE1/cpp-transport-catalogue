#pragma once

#include "geo.h"
#include "svg.h"
#include "graph.h"

#include <string>
#include <vector>
#include <string_view>
#include <variant>

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

    bool is_roundtrip;
};

}