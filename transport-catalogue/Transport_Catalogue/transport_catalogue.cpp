#include "transport_catalogue.h"

#include <numeric>
#include <stdexcept>
#include <unordered_set>

namespace TransporCatalogueLib {

namespace CatalogueCore {

// Функция добавления сущности остановки
void TransporCatalogue::AddStop(Domain::Stop &&stop) {
  // Добавляем остановку в хранилище
  stops_.push_back(std::move(stop));
  // Создаем ссылку на только что добвленную остановку
  Domain::Stop *buff = &stops_.back();
  // Добавляем ссулку на остановку в хранилище для быстрого доступа по имени
  // этой же остановки
  fast_stop_get_.insert({buff->stop_name, buff});
}

// Функция добавления сущности автобуса(маршрута)
void TransporCatalogue::AddBus(Domain::Bus &&bus) {
  // Добавляем автобус(маршрут) в хранилище
  buses_.push_back(std::move(bus));
  // Создаем ссылку на только что добвленный автобус(маршрут)
  Domain::Bus *buff = &buses_.back();
  // Добавляем ссулку на автобус(маршрут) в хранилище для быстрого доступа по
  // имени этого же автобуса(маршрута)
  fast_bus_get_.insert({buff->bus, buff});
  // Проходимся по списку остановок на маршруте(автобусе)
  for (auto &stop : buff->stops_for_bus_) {
    // Для остановки, на данном маршруте(автобусе), добавляем в список
    // маршрутов(автобусов), только что добавленный маршрут(автобус)
    fast_stop_get_.at(stop->stop_name)->routs.push_back(buff);
  }
}

// Функция поиска сущности остановки по ее имени
const Domain::Stop *
TransporCatalogue::FindStop(std::string_view stop_name) const {
  try {
    // При наличии остановки с заданным именем, выдаем на нее ссылку
    return fast_stop_get_.at(stop_name);
  } catch (const std::out_of_range &e) {
    // При отсутствии остановки с заданным именем, выдаем пустое значение
    return nullptr;
  }
}

// Функция поиска сущности автобуса(маршрута) по его имени
const Domain::Bus *TransporCatalogue::FindBus(std::string_view bus_name) const {
  try {
    // При наличии автобуса(маршрута) с заданным именем, выдаем на него ссылку
    return fast_bus_get_.at(bus_name);
  } catch (const std::out_of_range &e) {
    // При отсутствии автобуса(маршрута) с заданным именем, выдаем пустое
    // значение
    return nullptr;
  }
}

// Функция подсчета дистанции между всеми остановкими на маршруте(автобусе) по
// гео данным
static const double GeoDistanceBetweenStops(const Domain::Bus *bus) {
  return std::transform_reduce(
      std::next(bus->stops_for_bus_.begin()), bus->stops_for_bus_.end(),
      bus->stops_for_bus_.begin(), 0.0, std::plus<>{},
      [](const Domain::Stop *lhs, const Domain::Stop *rhs) {
        return Geo::ComputeDistance(
            {(*lhs).coordinates.lat, (*lhs).coordinates.lng},
            {(*rhs).coordinates.lat, (*rhs).coordinates.lng});
      });
}

// Функция подсчета дистанции между всеми остановкими на маршруте(автобусе) по
// реальным данным(дорогам)
static const double RouteDistance(const Domain::Bus *bus,
                                  const StopToStopDist &distances) {
  // Пустое значение для выдачи финального результата функции
  size_t to_ret = 0;
  // Копируем в контейнер список остановок для маршрута(автобуса)
  std::vector<const Domain::Stop *> route = bus->stops_for_bus_;
  // Итерируемся по списку остановок до мемента итерации на предпоследнюю
  // остановку
  for (size_t i = 0; i < bus->stops_for_bus_.size() - 1; ++i) {
    try {
      // Проверяем наличие дистанции от данной остановки до следующей и
      // прибавляем к результату
      to_ret += distances.at(route[i]->stop_name).at(route[i + 1]->stop_name);
    } catch (const std::out_of_range &e) {
      // При провале прошлого шага, ищем дистанцию для обратного порядка(от
      // следующей к данной)
      to_ret += distances.at(route[i + 1]->stop_name).at(route[i]->stop_name);
    }
  }
  return to_ret;
}

// Функция получения количества остановок на маршруте(автобусе)
static const size_t UniqueStopsOnBus(const Domain::Bus *bus) {
  // Пустое значение для выдачи финального результата функции
  std::unordered_set<const Domain::Stop *> to_ret;
  // Перекидываем список ссылок остановок в возвращаемое значение
  to_ret.insert(bus->stops_for_bus_.begin(), bus->stops_for_bus_.end());
  // Возвращаем размер ссылок на остановки
  return to_ret.size();
}

const TransporCatalogue::BusInfo
TransporCatalogue::GetBusInfo(const Domain::Bus *bus) const {
  return {GeoDistanceBetweenStops(bus), RouteDistance(bus, from_stop_to_stop_),
          UniqueStopsOnBus(bus)};
}

// Функция получения маршрутов(автобусов) для остановки
const std::set<std::string_view>
TransporCatalogue::GetRoutsOnStop(std::string_view stop_name) const {
  // Пустое значение для выдачи финального результата функции
  std::set<std::string_view> to_ret;
  // Итерируемся по списку маршрутов(автобусов), для заданной остановки
  for (const auto &bus : FindStop(stop_name)->routs) {
    // Добавляем имена автобусов(маршрутов) в возвращаемое значение
    to_ret.insert(bus->bus);
  }
  return to_ret;
}

// Функция добавления реальной дистанции(по дорогам) для остановок (остановка ->
// {остановка -> расстояние})
void TransporCatalogue::SetStopToStopDistance(std::string_view stop_from,
                                              std::string_view stop_to,
                                              size_t lenght) {
  from_stop_to_stop_[FindStop(stop_from)->stop_name]
                    [FindStop(stop_to)->stop_name] = lenght;
}

size_t
TransporCatalogue::GetDistanceBetweenStops(const Domain::Stop *from,
                                           const Domain::Stop *to) const {
  if (from_stop_to_stop_.count(from->stop_name) &&
      from_stop_to_stop_.at(from->stop_name).count(to->stop_name)) {
    return from_stop_to_stop_.at(from->stop_name).at(to->stop_name);
  } else if (from_stop_to_stop_.count(to->stop_name) &&
             from_stop_to_stop_.at(to->stop_name).count(from->stop_name)) {
    return from_stop_to_stop_.at(to->stop_name).at(from->stop_name);
  }
  return 0;
}

} // namespace CatalogueCore

} // namespace TransporCatalogueLib
