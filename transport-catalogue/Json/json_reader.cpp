#include "json_reader.h"
#include "../SVG/svg.h"
#include "../Transport_Catalogue/domain.h"
#include "json_builder.h"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace TransporCatalogueLib {

namespace jsonOperator {

struct EdgeBuilder {
  json::Node operator()(const TransportRouter::StopEdge &edge) {
    json::Builder to_ret;
    to_ret.StartDict()
        .Key("type")
        .Value("Wait")
        .Key("stop_name")
        .Value(std::string(edge.stop_name))
        .Key("time")
        .Value(edge.time)
        .EndDict();
    return to_ret.Build();
  }

  json::Node operator()(const TransportRouter::BusEdge &edge) {
    json::Builder to_ret;
    to_ret.StartDict()
        .Key("type")
        .Value("Bus")
        .Key("bus")
        .Value(edge.bus_name)
        .Key("span_count")
        .Value(int(edge.span))
        .Key("time")
        .Value(edge.time)
        .EndDict();
    return to_ret.Build();
  }
};

// Получаем информацию на добавление с команды base_requests
void ParseBase(
    json::Node &root, Serialization::SerializationClass &serialize,
    CatalogueCore::TransporCatalogue &cat /*, Handler::InfoKeeper& keeper*/) {
  // Структура для хранения дистанции между остановками
  struct DistanceToAdd {
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
  for (json::Node &request : requests) {
    // Получаем информацию из команды
    json::Dict req = request.AsMap();
    // Определяем тип команды
    json::Node command = req.at("type");
    if (command.AsString() == "Bus") {
      // Если команда - Bus, добавляем в очередь
      bus_queue.push_back(request);
    } else if (command.AsString() == "Stop") {
      // Если команда - Stop, добавляем в очередь
      stop_queue.push_back(request);
    }
  }
  // Проходимся по командам Stop
  for (json::Node &stop_n : stop_queue) {
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
    // keeper.SaveStop(stop_name);
    // Проверяемналичие обозначения дистанций в команде
    if (stop.count("road_distances")) {
      // Получаем информацию о дистанциях
      json::Dict distances = stop.at("road_distances").AsMap();
      // Проходимся по информации о дистанциях
      for (auto [to, distance] : distances) {
        // Добавляем дистанции в очередь
        distance_queue.push_back({stop_name, to, distance.AsInt()});
      }
    }
  }
  // Проходимся по очереди на добавление дистанций
  for (DistanceToAdd distances : distance_queue) {
    // Сохраняем дистанции в каталог
    cat.SetStopToStopDistance(distances.from, distances.to, distances.distance);
  }
  // Проходимся по очереди на добавление автобусов(маршрутов)
  for (json::Node &bus_n : bus_queue) {
    // Получаем информацию о маршруте(автобусе)
    json::Dict bus = bus_n.AsMap();
    // Название автобуса(маршрута)
    std::string bus_name = bus.at("name").AsString();
    // Хранилище для остановок на маршруте(автобусе)
    std::vector<const Domain::Stop *> stops_for_bus;
    // Получаем информацию об остановках, на маршруте(автобусе)
    json::Array stops = bus.at("stops").AsArray();
    // Проходимся по остановкам
    for (json::Node &stop : stops) {
      // Добавляем остановки в хранилище
      stops_for_bus.push_back(cat.FindStop(stop.AsString()));
    }
    // Проверяем является ли маршрут(автобус) не круговым замкнутым
    if (!bus.at("is_roundtrip").AsBool()) {
      // Создаем копию хранилища остановок
      std::vector<const Domain::Stop *> route_back;
      // Вставляем в копию остановки, в обратном порядке, исключая конечную
      route_back.insert(route_back.end(), std::next(stops_for_bus.rbegin()),
                        stops_for_bus.rend());
      // Вставляем в конец оригинального хранилища копию
      stops_for_bus.insert(stops_for_bus.end(), route_back.begin(),
                           route_back.end());
    }
    // Добавляем автобус(маршрут) в каталог
    cat.AddBus({bus_name, stops_for_bus, bus.at("is_roundtrip").AsBool()});
    // Сохраняем информацию о маршруте(автобусе) в удобное для взаимодействия
    // хранилище
    // keeper.SaveBus(bus_name, bus.at("is_roundtrip").AsBool());
  }
  serialize.SerializeCatalogue(cat);
}

// Функция проверяет тип задаваемого цвета
svg::Color CheckAndAddColor(json::Node &raw_color) {
  if (raw_color.IsString()) {
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
  if (color_palette.size() == 4) {
    // Сохраняем значение прозрачности
    double opacity = color_palette[3].AsDouble();
    // Если цвет задается RGBA типом
    return svg::Color(svg::Rgba(red, green, blue, opacity));
  }
  // Если цвет задается RGB типом
  return svg::Color(svg::Rgb(red, green, blue));
}

// Получаем информацию для визуализации карты с команды render_settings
void ParseRender(json::Node &root,
                 Serialization::SerializationClass &serialize) {
  // Преаброзуем команду в тип Dict
  json::Dict raw_settings = root.AsMap();
  // Объект для сохранения результата
  map_render::RenderSettings to_ret;
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
  to_ret.underlayer_color =
      CheckAndAddColor(raw_settings.at("underlayer_color"));
  // Получаем список цветов для использования на линиях и названиях
  // маршрутов(автобусов)
  for (json::Node color : raw_settings.at("color_palette").AsArray()) {
    to_ret.color_palette.push_back(CheckAndAddColor(color));
  }
  serialize.SerializeRenderSettings(to_ret);
}

void ParseRouting(json::Node &root, CatalogueCore::TransporCatalogue &cat,
                  Serialization::SerializationClass
                      &serialize /*, Handler::InfoKeeper& keeper*/) {
  TransportRouter::Router router;
  // Получаем Dict настроек движения автобусов
  json::Dict settings = root.AsMap();
  // Добавляем настройки в удобное для взаимодействия хранилище
  std::vector<const Domain::Stop *> stops;
  // for (auto stop : keeper.GetAllStops())
  // {
  //     stops.push_back(cat.FindStop(stop));
  // }
  for (const auto &stop : cat.GetStops()) {
    stops.push_back(cat.FindStop(stop.stop_name));
  }
  std::vector<std::pair<const Domain::Bus *, bool>> buses;
  // for (auto bus : keeper.GetAllBuses())
  // {
  //     buses.push_back(std::make_pair(cat.FindBus(bus.first), bus.second));
  // }
  for (const auto &bus : cat.GetBuses()) {
    buses.push_back(std::make_pair(cat.FindBus(bus.bus), bus.is_roundtrip));
  }
  router.SetBusSettings({settings.at("bus_wait_time").AsDouble(),
                         settings.at("bus_velocity").AsDouble()});
  router.SetRouter(stops, buses, cat);
  serialize.SerializeRouting(router);
}

// Получаем информацию для вывода с команды stat_requests
std::vector<json::Node> ParseStat(json::Node &root,
                                  const CatalogueCore::TransporCatalogue &cat,
                                  map_render::MapRender &map,
                                  TransportRouter::Router &router) {
  // Список команд на вывод
  json::Array requests = root.AsArray();
  // Список готовых ответов на вывод
  std::vector<json::Node> to_print;
  to_print.reserve(requests.size());
  // Проходимся по запросам
  for (json::Node &request_n : requests) {
    // Информация запроса
    json::Dict request = request_n.AsMap();
    // Получаем тип запроса
    std::string type = request.at("type").AsString();
    // Если тип запроса Bus
    if (type == "Bus") {
      // Получаем автобус(маршрут)
      auto bus = cat.FindBus(request.at("name").AsString());
      // Объект для построения результата
      json::Builder to_emplace;
      // Если автобуса(маршрута) не существует
      if (bus == nullptr) {
        to_emplace.StartDict()
            .Key("request_id")
            .Value(request.at("id").AsInt())
            .Key("error_message")
            .Value("not found")
            .EndDict();
      }
      // Если автобус(маршрут) существует
      else {
        auto bus_info = cat.GetBusInfo(bus);
        double curvature =
            bus_info.route_distance / bus_info.geo_distance_between_stops;
        to_emplace.StartDict()
            .Key("curvature")
            .Value(curvature)
            .Key("request_id")
            .Value(request.at("id").AsInt())
            .Key("route_length")
            .Value(bus_info.route_distance)
            .Key("stop_count")
            .Value(int(bus->stops_for_bus_.size()))
            .Key("unique_stop_count")
            .Value(int(bus_info.unique_stops_on_bus))
            .EndDict();
      }
      // Добавить собранный объект в вывод
      to_print.push_back(to_emplace.Build());
    }
    // Если тип запроса Stop
    else if (type == "Stop") {
      // Объект для построения результата
      json::Builder to_emplace;
      // Если остановка не существует
      if (!cat.FindStop(request.at("name").AsString())) {
        to_emplace.StartDict()
            .Key("request_id")
            .Value(request.at("id").AsInt())
            .Key("error_message")
            .Value("not found")
            .EndDict();
      }
      // Если остановка существует
      else {
        json::Array buses;
        for (std::string_view bus :
             cat.GetRoutsOnStop(request.at("name").AsString())) {
          buses.push_back(json::Node(std::string(bus)));
        }
        to_emplace.StartDict()
            .Key("buses")
            .Value(buses)
            .Key("request_id")
            .Value(request.at("id").AsInt())
            .EndDict();
      }
      // Добавить собранный объект в вывод
      to_print.push_back(to_emplace.Build());
    }
    // Если тип запроса Map
    else if (type == "Map") {
      // Получаем собранную карту
      std::ostringstream get_map = map.Draw(cat.GetBuses(), cat.GetStops());
      std::string map_to_json = get_map.str();
      // Объект для построения результата
      json::Builder map_data;
      map_data.StartDict()
          .Key("map")
          .Value(map_to_json)
          .Key("request_id")
          .Value(request.at("id").AsInt())
          .EndDict();
      // Добавить собранный объект в вывод
      to_print.push_back(map_data.Build());
    } else if (type == "Route") {
      const auto &route =
          router.BuildAndGetRoute(cat.FindStop(request.at("from").AsString()),
                                  cat.FindStop(request.at("to").AsString()));
      json::Builder to_emplace;
      if (!route) {
        to_emplace.StartDict()
            .Key("request_id")
            .Value(request.at("id").AsInt())
            .Key("error_message")
            .Value("not found")
            .EndDict();
      } else {
        json::Array items;
        for (const auto &item : route->route_edges) {
          items.emplace_back(std::visit(EdgeBuilder{}, item));
        }
        to_emplace.StartDict()
            .Key("request_id")
            .Value(request.at("id").AsInt())
            .Key("total_time")
            .Value(route->time)
            .Key("items")
            .Value(items)
            .EndDict();
      }
      to_print.push_back(to_emplace.Build());
    }
  }
  return to_print;
}

void Reader(Serialization::SerializationClass &serialize,
            Handler::InfoKeeper &keeper, const bool serialization) {
  // Получаем нод входных данных
  json::Node root = keeper.GetInput().GetRoot();
  // Преобразуем нод в map, где в качестве ключей - тип команды на исполнение
  json::Dict commands = root.AsMap();
  if (serialization) {
    CatalogueCore::TransporCatalogue cat;

    try {
      // Пытаемся получить информацию на добавление с команды base_requests
      ParseBase(commands.at("base_requests"), serialize, cat);
    } catch (...) {
    }
    try {
      // Пытаемся получить информацию для визуализации карты с команды
      // render_settings
      ParseRender(commands.at("render_settings"), serialize);
    } catch (...) {
    }
    try {
      // Пытаемся получить информацию о движении автобусов
      ParseRouting(commands.at("routing_settings"), cat, serialize);
    } catch (...) {
    }
  } else {
    CatalogueCore::TransporCatalogue cat = serialize.DeserializeCatalogue();
    map_render::MapRender drawing;
    *drawing.MutableGetSettings() = serialize.DeserializeRenderSettings();
    TransportRouter::Router routing = serialize.DeserializeRouting(cat);

    // Вектор выводов для запросов
    std::vector<json::Node> to_output;

    try {
      // Пытаемся получить информацию для вывода с команды stat_requests
      to_output =
          ParseStat(commands.at("stat_requests"), cat, drawing, routing);
    } catch (...) {
    }
    // Добавляем в хранилище информацию на вывод
    keeper.GetOutput() = json::Document(json::Node(to_output));
  }
}

} // namespace jsonOperator

} // namespace TransporCatalogueLib
