#pragma once

#include "geo.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <deque>
#include <set>
#include <vector>

namespace TransporCatalogueLib
{

namespace CatalogueCore
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

typedef std::unordered_map<std::string_view, std::unordered_map<std::string_view, size_t>> StopToStopDist;

// Класс - ядро
class TransporCatalogue {
public:

    // Функции добавления сущностей остановка/автобус(маршрут)
    void AddStop(Stop&& stop);
    void AddBus(Bus&& bus);

    // Функции поиска сущностей остановка/автобус(маршрут)
    const Stop* FindStop(std::string_view stop) const;
    const Bus* FindBus(std::string_view bus_name) const;

    // Структура нужной для вывода информации о маршруте(автобусе)
    struct BusInfo
    {
        double geo_distance_between_stops = 0.0;
        double route_distance = 0.0;
        size_t unique_stops_on_bus = 0;
    };

    // Функция получения нужной информации о маршруте(автобусе)
    const BusInfo GetBusInfo(const Bus* bus) const;
    
    // Функция получения маршрутов(автобусов) для остановки
    const std::set<std::string_view> GetRoutsOnStop(std::string_view stop_name) const;

    // Функция добавления реальной дистанции(по дорогам) для остановок (остановка -> {остановка -> расстояние})
    void SetStopToStopDistance(std::string_view stop_from, std::string_view stop_to, size_t lenght);

private:

    // Хранилище остановок
    std::deque<Stop> stops_;
    // Быстрый доступ к хранилищю остановок
    std::unordered_map<std::string_view, Stop*> fast_stop_get_;
    // Хранилище автобусов(маршрутов)
    std::deque<Bus> buses_;
    // Быстрый доступ к хранилищю автобусов(маршрутов)
    std::unordered_map<std::string_view, Bus*> fast_bus_get_;
    // Хранилище реальной дистанции(по дорогам) между остановками (остановка -> {остановка -> расстояние})
    StopToStopDist from_stop_to_stop_;

};

}

}