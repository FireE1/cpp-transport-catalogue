#pragma once

#include <cmath>

namespace Geo
{

// Структура координат
struct Coordinates {
    // Широта
    double lat;
    // Долгота
    double lng;

    // Объявление операций сравнивая
    bool operator==(const Coordinates& other) const { // Координаты равны
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const { // Координаты не равны
        return !(*this == other);
    }
};

// Вычисление расстояния по координатам
inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    // Если координаты равны - расстояние = нулю
    if (from == to) 
    {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}

}