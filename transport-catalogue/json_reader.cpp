#include "json_reader.h"
#include "svg.h"
#include "domain.h"

#include <vector>
#include <iostream>
#include <utility>
#include <string>
#include <sstream>

namespace TransporCatalogueLib
{

namespace jsonOperator
{

// Получаем информацию на добавление с команды base_requests
void ParseBase(json::Node& root, CatalogueCore::TransporCatalogue& cat, Handler::InfoKeeper& keeper) {

    // Структура для хранения дистанции между остановками
    struct DistanceToAdd
    {
        std::string from;
        std::string to;
        int distance;
    };
    
    // Список команд на добавление
    json::Array requests = root.AsArray();
    // Очередь добавления автобусов(маршрутов)
    std::vector<json::Node> bus_queue;
    // Очередь добавления остановок
    std::vector<json::Node> stop_queue;
    // Очередь добавления дистанций между остановками
    std::vector<DistanceToAdd> distance_queue;
    // Проходимся по списку команд
    for (json::Node& request : requests)
    {
        // Получаем информацию из команды
        json::Dict req = request.AsMap();
        // Определяем тип команды
        json::Node command = req.at("type");
        if (command.AsString() == "Bus")
        {
            // Если команда - Bus, добавляем в очередь
            bus_queue.push_back(request);
        }
        else if (command.AsString() == "Stop")
        {
            // Если команда - Stop, добавляем в очередь
            stop_queue.push_back(request);
        }
    }
    // Проходимся по командам Stop
    for (json::Node& stop_n : stop_queue)
    {
        // Получаем информацию с команды
        json::Dict stop = stop_n.AsMap();
        // Название остановки
        std::string stop_name = stop.at("name").AsString();
        // Широта
        double latitude = stop.at("latitude").AsDouble();
        // Долгота
        double longitude = stop.at("longitude").AsDouble();
        // Добавляем остановку в каталог
        cat.AddStop({stop_name, {latitude, longitude}, {}});
        // Сохраняем информацию об остановке в удобное для взаимодействия хранилище
        keeper.SaveStop(stop_name);
        // Проверяемналичие обозначения дистанций в команде
        if (stop.count("road_distances"))
        {
            // Получаем информацию о дистанциях
            json::Dict distances = stop.at("road_distances").AsMap();
            // Проходимся по информации о дистанциях
            for (auto [to, distance] : distances)
            {
                // Добавляем дистанции в очередь
                distance_queue.push_back({stop_name, to, distance.AsInt()});
            }
        }
    }
    // Проходимся по очереди на добавление дистанций
    for (DistanceToAdd distances : distance_queue)
    {
        // Сохраняем дистанции в каталог
        cat.SetStopToStopDistance(distances.from, distances.to, distances.distance);
    }
    // Проходимся по очереди на добавление автобусов(маршрутов)
    for (json::Node& bus_n : bus_queue)
    {
        // Получаем информацию о маршруте(автобусе)
        json::Dict bus = bus_n.AsMap();
        // Название автобуса(маршрута)
        std::string bus_name = bus.at("name").AsString();
        // Хранилище для остановок на маршруте(автобусе)
        std::vector<const Domain::Stop*> stops_for_bus;
        // Получаем информацию об остановках, на маршруте(автобусе)
        json::Array stops = bus.at("stops").AsArray();
        // Проходимся по остановкам
        for (json::Node& stop : stops)
        {
            // Добавляем остановки в хранилище
            stops_for_bus.push_back(cat.FindStop(stop.AsString()));
        }
        // Проверяем является ли маршрут(автобус) не круговым замкнутым
        if (!bus.at("is_roundtrip").AsBool())
        {
            // Создаем копию хранилища остановок
            std::vector<const Domain::Stop*> route_back;
            // Вставляем в копию остановки, в обратном порядке, исключая конечную
            route_back.insert(route_back.end(), std::next(stops_for_bus.rbegin()), stops_for_bus.rend());
            // Вставляем в конец оригинального хранилища копию
            stops_for_bus.insert(stops_for_bus.end(), route_back.begin(), route_back.end());
        }
        // Добавляем автобус(маршрут) в каталог
        cat.AddBus({bus_name, stops_for_bus});
         // Сохраняем информацию о маршруте(автобусе) в удобное для взаимодействия хранилище
        keeper.SaveBus(bus_name, bus.at("is_roundtrip").AsBool());
    }
}

// Функция проверяет тип задаваемого цвета
svg::Color CheckAndAddColor(json::Node& raw_color) {
    if (raw_color.IsString())
    {
        // Если цвет задается строкой
        return svg::Color(std::string(raw_color.AsString()));
    }
    // Переводим запись цвета в Array
    json::Array color_palette = raw_color.AsArray();
    // Сохраняем значение красного
    int red = color_palette[0].AsInt();
    // Сохраняем значение зеленого
    int green = color_palette[1].AsInt();
    // Сохраняем значение синего
    int blue = color_palette[2].AsInt();
    // Проверяем наличие четвертого обозначения
    if (color_palette.size() == 4)
    {
        // Сохраняем значение прозрачности
        double opacity = color_palette[3].AsDouble();
        // Если цвет задается RGBA типом
        return svg::Color(svg::Rgba(red, green, blue, opacity));
    }
    // Если цвет задается RGB типом
    return svg::Color(svg::Rgb(red, green, blue));
}

// Получаем информацию для визуализации карты с команды render_settings
map_render::MapRender ParseRender(json::Node& root) {
    // Преаброзуем команду в тип Dict
    json::Dict raw_settings = root.AsMap();
    // Объект для сохранения результата
    Domain::RenderSettings to_ret;
    // Получаем широту полотна
    to_ret.width = raw_settings.at("width").AsDouble();
    // Получаем высоту полотна
    to_ret.height = raw_settings.at("height").AsDouble();
    // Получаем значение отступа
    to_ret.padding = raw_settings.at("padding").AsDouble();
    // Получаем толщину линий
    to_ret.line_width = raw_settings.at("line_width").AsDouble();
    // Получаем радиус точек остановок
    to_ret.stop_radius = raw_settings.at("stop_radius").AsDouble();
    // Получаем размер шрифта для маршрутов(автобусов)
    to_ret.bus_label_font_size = raw_settings.at("bus_label_font_size").AsInt();
    // Получаем отступ для маршрутов(автобусов)
    json::Array bus_off = raw_settings.at("bus_label_offset").AsArray();
    to_ret.bus_label_offset.x = bus_off[0].AsDouble();
    to_ret.bus_label_offset.y = bus_off[1].AsDouble();
    // Получаем размер шрифта для остановок
    to_ret.stop_label_font_size = raw_settings.at("stop_label_font_size").AsInt();
    // Получаем отступ для остановок
    json::Array stop_off = raw_settings.at("stop_label_offset").AsArray();
    to_ret.stop_label_offset.x = stop_off[0].AsDouble();
    to_ret.stop_label_offset.y = stop_off[1].AsDouble();
    // Получаем толщину подложки
    to_ret.underlayer_width = raw_settings.at("underlayer_width").AsDouble();
    // Получаем цвет подложки
    to_ret.underlayer_color = CheckAndAddColor(raw_settings.at("underlayer_color"));
    // Получаем список цветов для использования на линиях и названиях маршрутов(автобусов)
    for (json::Node color : raw_settings.at("color_palette").AsArray())
    {
        to_ret.color_palette.push_back(CheckAndAddColor(color));
    }
    // Возвращаем объект для проецирования карты с заданными настройками
    return map_render::MapRender(to_ret);
}

// Получаем информацию для вывода с команды stat_requests
std::vector<json::Node> ParseStat(json::Node& root, const CatalogueCore::TransporCatalogue& cat, map_render::MapRender& map, Handler::InfoKeeper& keeper) {
    // Список команд на вывод
    json::Array requests = root.AsArray();
    // Список готовых ответов на вывод
    std::vector<json::Node> to_print;
    to_print.reserve(requests.size());
    // Проходимся по запросам
    for (json::Node& request_n : requests)
    {
        // Информация запроса
        json::Dict request = request_n.AsMap();
        // Получаем тип запроса
        std::string type = request.at("type").AsString();
        // Если тип запроса Bus
        if (type == "Bus")
        {
            // Получаем автобус(маршрут)
            auto bus = cat.FindBus(request.at("name").AsString());
            // Объект для сохранения результата
            json::Dict to_emplace;
            // Если автобуса(маршрута) не существует
            if (bus == nullptr)
            {
                to_emplace.emplace("request_id", json::Node(request.at("id").AsInt()));
                to_emplace.emplace("error_message", json::Node(std::string("not found")));
            }
            // Если автобус(маршрут) существует
            else
            {
                auto bus_info = cat.GetBusInfo(bus);
                double curvature = bus_info.route_distance / bus_info.geo_distance_between_stops;
                to_emplace.emplace("curvature", json::Node(double(curvature)));
                to_emplace.emplace("request_id", json::Node(request.at("id").AsInt()));
                to_emplace.emplace("route_length", json::Node(double(bus_info.route_distance)));
                to_emplace.emplace("stop_count", json::Node(int(bus->stops_for_bus_.size())));
                to_emplace.emplace("unique_stop_count", json::Node(int(bus_info.unique_stops_on_bus)));
            }
            // Добавить собранный объект в вывод
            to_print.push_back(json::Node(to_emplace));
        }
        // Если тип запроса Stop
        else if (type == "Stop")
        {
            // Объект для сохранения результата
            json::Dict to_emplace;
            // Если остановка не существует
            if (!cat.FindStop(request.at("name").AsString()))
            {
                to_emplace.emplace("request_id", json::Node(request.at("id").AsInt()));
                to_emplace.emplace("error_message", json::Node(std::string("not found")));
            }
            // Если остановка существует
            else
            {
                json::Array buses;
                for (std::string_view bus : cat.GetRoutsOnStop(request.at("name").AsString()))
                {
                    buses.push_back(json::Node(std::string(bus)));
                }
                to_emplace.emplace("buses", json::Node(buses));
                to_emplace.emplace("request_id", json::Node(request.at("id").AsInt()));
            }
            // Добавить собранный объект в вывод
            to_print.push_back(json::Node(to_emplace));
        }
        // Если тип запроса Map
        else if (type == "Map")
        {
            // Получаем собранную карту
            std::ostringstream get_map = map.Draw(cat, keeper.GetAllBuses(), keeper.GetAllStops());
            std::string map_to_json = get_map.str();
            // Объект для сохранения результата
            json::Dict map_data;
            map_data.emplace("map", json::Node(map_to_json));
            map_data.emplace("request_id", json::Node(request.at("id").AsInt()));
            // Добавить собранный объект в вывод
            to_print.push_back(json::Node(map_data));
        }
    }
    return to_print;
}

void Reader(Handler::InfoKeeper& keeper, CatalogueCore::TransporCatalogue& cat, map_render::MapRender& map) {
    // Получаем нод входных данных
    json::Node root = keeper.GetInput().GetRoot();
    // Преобразуем нод в map, где в качестве ключей - тип команды на исполнение
    json::Dict commands = root.AsMap();
    // Вектор выводов для запросов
    std::vector<json::Node> to_output;
    try
    {
        // Пытаемся получить информацию на добавление с команды base_requests
        ParseBase(commands.at("base_requests"), cat, keeper);
    }
    catch(...){}
    try
    {
        // Пытаемся получить информацию для визуализации карты с команды render_settings
        map = ParseRender(commands.at("render_settings"));
    }
    catch(...){}
    try
    {
        // Пытаемся получить информацию для вывода с команды stat_requests
        to_output = ParseStat(commands.at("stat_requests"), cat, map, keeper);
    }
    catch(...){}
    // Добавляем в хранилище информацию на вывод
    keeper.GetOutput() = json::Document(json::Node(to_output));
}

}

}