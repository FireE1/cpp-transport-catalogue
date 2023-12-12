#pragma once

namespace Geo {

// Структура координат
struct Coordinates {
  // Широта
  double lat;
  // Долгота
  double lng;

  // Объявление операций сравнивая
  bool operator==(const Coordinates &other) const { // Координаты равны
    return lat == other.lat && lng == other.lng;
  }
  bool operator!=(const Coordinates &other) const { // Координаты не равны
    return !(*this == other);
  }
};

// Вычисление расстояния по координатам
double ComputeDistance(Coordinates from, Coordinates to);

} // namespace Geo
