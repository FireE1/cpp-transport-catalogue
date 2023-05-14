#pragma once

#include "geo.h"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <algorithm>
#include <list>
#include <numeric>
#include <set>
#include <stdexcept>
#include <vector>

namespace CatalogueCore
{

// Структура автобуса(маршрута)
struct Bus;

// Структура остановки
struct Stop {
    // Название остановки
    std::string stop_name;
    // Широта по гео данным
    double latitude;
    // Долгота по гео данным
    double longitude;
    // Отсортированное хранилище маршрутов(автобусов), проходящих через остановку
    std::set<Bus*>routs;
};

// Структура автобуса(маршрута)
struct Bus {
    // Название пвтобуса(маршрута)
    std::string bus;
    // Хранилище ссылок на остановки, находящиеся на маршруте(автобусе)
    std::list<Stop*> stops_for_bus_;
};

// Класс - ядро
class TransporCatalogue {
public:

    // Функции добавления сущностей остановка/автобус(маршрут)
    void AddStop(Stop&& stop);
    void AddBus(Bus&& bus);

    // Функции поиска сущностей остановка/автобус(маршрут)
    Stop* FindStop(std::string_view stop);
    Bus* FindBus(std::string_view bus_name);

    // Функции получения дистанций между остановок
    double GeoDistanceBetweenStops(const Bus* bus); // Дистанция по гео данным
    size_t RouteDistance(const Bus* bus);           // Дистанция по реальному пути(дорогам)
    
    // Функция получения количества остановок на маршруте(автобусе)
    size_t UniqueStopsOnBus(const Bus* bus);

    // Функция получения маршрутов(автобусов) для остановки
    std::set<std::string_view> GetRoutsOnStop(std::string_view stop_name);

    // Функция добавления реальной дистанции(по дорогам) для остановок (остановка -> {остановка -> расстояние})
    void AddStopToStopLenght (std::string_view stop_from, std::string_view stop_to, size_t& lenght);
                            

private:

    // Хранилище остановок
    std::list<Stop> stops_;
    // Быстрый доступ к хранилищю остановок
    std::unordered_map<std::string_view, Stop*> fast_stop_get_;
    // Хранилище автобусов(маршрутов)
    std::list<Bus> buses_;
    // Быстрый доступ к хранилищю автобусов(маршрутов)
    std::unordered_map<std::string_view, Bus*> fast_bus_get_;
    // Хранилище реальной дистанции(по дорогам) между остановками (остановка -> {остановка -> расстояние})
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, size_t>> from_stop_to_stop_;

};

}