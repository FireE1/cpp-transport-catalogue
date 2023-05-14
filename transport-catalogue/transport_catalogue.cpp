#include "transport_catalogue.h"

namespace CatalogueCore
{

// Функция добавления сущности остановки
void TransporCatalogue::AddStop(Stop&& stop) {
    // Добавляем остановку в хранилище
    stops_.push_back(std::move(stop));
    // Создаем ссылку на только что добвленную остановку
    Stop* buff = &stops_.back();
    // Добавляем ссулку на остановку в хранилище для быстрого доступа по имени этой же остановки
    fast_stop_get_.insert({buff->stop_name, buff});
}

// Функция добавления сущности автобуса(маршрута)
void TransporCatalogue::AddBus(Bus&& bus) {
    // Добавляем автобус(маршрут) в хранилище
    buses_.push_back(std::move(bus));
    // Создаем ссылку на только что добвленный автобус(маршрут)
    Bus* buff = &buses_.back();
    // Добавляем ссулку на автобус(маршрут) в хранилище для быстрого доступа по имени этого же автобуса(маршрута)
    fast_bus_get_.insert({buff->bus, buff});
    // Проходимся по списку остановок на маршруте(автобусе)
    for (auto& stop : buff->stops_for_bus_)
    {
        // Для остановки, на данном маршруте(автобусе), добавляем в список маршрутов(автобусов), только что добавленный маршрут(автобус)
        FindStop(stop->stop_name)->routs.insert(buff);
    }
}

// Функция поиска сущности остановки по ее имени
Stop* TransporCatalogue::FindStop(std::string_view stop_name) {
    try 
    {
        // При наличии остановки с заданным именем, выдаем на нее ссылку
        return fast_stop_get_.at(stop_name);
    }
    catch (const std::out_of_range &e)
    {
        // При отсутствии остановки с заданным именем, выдаем пустое значение
        return {};
    }
}

// Функция поиска сущности автобуса(маршрута) по его имени
Bus* TransporCatalogue::FindBus(std::string_view bus_name) {
    try 
    {
        // При наличии автобуса(маршрута) с заданным именем, выдаем на него ссылку
        return fast_bus_get_.at(bus_name);
    }
    catch (const std::out_of_range &e)
    {
        // При отсутствии автобуса(маршрута) с заданным именем, выдаем пустое значение
        return {};
    }
}

// Функция подсчета дистанции между всеми остановкими на маршруте(автобусе) по гео данным
double TransporCatalogue::GeoDistanceBetweenStops(const Bus* bus) {
    return std::transform_reduce(std::next(bus->stops_for_bus_.begin()), bus->stops_for_bus_.end(),
            bus->stops_for_bus_.begin(), 0.0, std::plus<>{}, [](const Stop* lhs, const Stop* rhs){
                return Geo::ComputeDistance({(*lhs).latitude, (*lhs).longitude},
                {(*rhs).latitude, (*rhs).longitude});
            });
}

// Функция подсчета дистанции между всеми остановкими на маршруте(автобусе) по реальным данным(дорогам)
size_t TransporCatalogue::RouteDistance(const Bus* bus) {
    // Пустое значение для выдачи финального результата функции
    size_t to_ret = 0;
    // Список остановок для маршрута(автобуса)
    std::vector<std::string_view> route;
    // Проходимся по списку ссылок на остановки для маршрута(автобуса)
    for (Stop* stop : bus->stops_for_bus_)
    {
        // Кладем имена остановок в более удобный для итерирования контейнер
        route.push_back(stop->stop_name);
    }
    // Итерируемся по списку остановок до мемента итерации на предпоследнюю остановку
    for (size_t i = 0; i < route.size() - 1; ++i)
    {
        try
        {
            // Проверяем наличие дистанции от данной остановки до следующей и прибавляем к результату
            to_ret += from_stop_to_stop_.at(route[i]).at(route[i + 1]);
        }
        catch (const std::out_of_range &e)
        {
            // При провале прошлого шага, ищем дистанцию для обратного порядка(от следующей к данной)
            to_ret += from_stop_to_stop_.at(route[i + 1]).at(route[i]);
        }
    }
    return to_ret;
}

// Функция получения количества остановок на маршруте(автобусе)
size_t TransporCatalogue::UniqueStopsOnBus(const Bus* bus) {
    // Пустое значение для выдачи финального результата функции
    std::unordered_set<const Stop*> to_ret;
    // Перекидываем список ссылок остановок в возвращаемое значение
    to_ret.insert(bus->stops_for_bus_.begin(), bus->stops_for_bus_.end());
    // Возвращаем размер ссылок на остановки
    return to_ret.size();
}

// Функция получения маршрутов(автобусов) для остановки
std::set<std::string_view> TransporCatalogue::GetRoutsOnStop(std::string_view stop_name) {
    // Пустое значение для выдачи финального результата функции
    std::set<std::string_view> to_ret;
    // Итерируемся по списку маршрутов(автобусов), для заданной остановки
    for (const auto& bus : FindStop(stop_name)->routs)
    {
        // Добавляем имена автобусов(маршрутов) в возвращаемое значение
        to_ret.insert(bus->bus);
    }
    return to_ret;
}

// Функция добавления реальной дистанции(по дорогам) для остановок (остановка -> {остановка -> расстояние})
void TransporCatalogue::AddStopToStopLenght (std::string_view stop_from, std::string_view stop_to, size_t& lenght) {
    from_stop_to_stop_[FindStop(stop_from)->stop_name][FindStop(stop_to)->stop_name] = lenght;
}

}