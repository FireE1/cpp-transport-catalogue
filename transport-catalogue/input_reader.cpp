#include "input_reader.h"

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>

namespace TransporCatalogueLib
{

namespace Input
{

// Добавляем остановку в каталог и возвращаем дистанцию для остановок (остановка , {остановка -> расстояние})
static std::pair<std::string, std::map<std::string, size_t>> QueueAddStop(std::string& text, CatalogueCore::TransporCatalogue& cat) {
    // Находим конец для названия остановки
    auto two_points = text.find_first_of(':');
    // Находим разделение между широтой и долготой остановки
    auto lat_long_devider = text.find_first_of(',');
    // string объект для хранения названия остановки
    std::string stop_name = text.substr(0, two_points);
    // double объект для хранения широты
    double latitude = std::stod(text.substr(two_points + 2, lat_long_devider));
    // Ищем вторую запятую в команде
    auto posible_distances_dev = text.find_first_of(',', lat_long_devider + 1);
    // double объект для хранения долготы
    double longitude = std::stod(text.substr(lat_long_devider + 2, posible_distances_dev));
    // Добавляем остановку в каталог
    cat.AddStop({stop_name, {latitude, longitude}, {}});
    // Создаем пустой объект для остановки и дистанции до нее
    std::map<std::string, size_t> distances_to_stops;
    // Находим начало обозначений дистанций
    auto distances_start = text.find_first_of(',', lat_long_devider + 1);
    // При наличии обозначений дистанций, выполняем ниже написанное
    if (posible_distances_dev < text.size())
    {
        for (;;)
        {
            // Переводим начало на первое число дистанции
            distances_start += 2;
            // Находим окончание обозначения дистанции
            auto distance_end = text.find_first_of('m', distances_start);
            // Сохраняем дистанцию в объект size_t
            size_t distance = std::stoul(text.substr(distances_start, distance_end - distances_start));
            // Находим начало названия остановки
            auto stop_to_name_start = text.find_first_of(' ', distance_end + 2);
            // Находим окончание названия остановки
            auto stop_to_name_end = text.find_first_of(',', stop_to_name_start + 1);
            // Сохраняем название остановки в объект string
            std::string stop_to_name = text.substr(stop_to_name_start + 1, stop_to_name_end - stop_to_name_start - 1);
            // Сохраняем дистанцию до остановки и ее название в объект map
            distances_to_stops[stop_to_name] = distance;
            // Если окончание названия остановки выходит за границы команды - останавливаем последующий поиск
            if (stop_to_name_end == text.npos)
            {
                break;
            }
            // Переводим начало на конец названия остановки
            distances_start = stop_to_name_end;
        }
    }
    return std::make_pair(stop_name, distances_to_stops);
}

// Проверяем тип проходимого маршрута автобусом
static char IsRound(std::string& stops_queue) {
    // Ищем разделение остановок в команде
    std::string cut = stops_queue.substr(stops_queue.find_first_of(' ') + 1, stops_queue.size());
    if (cut[0] == '-' || cut[0] == '>')
    {
        // Возвращаем знак разделения остановок в команде
        return cut[cut.find_first_not_of(' ')];
    }
    // При неудаче поиска знака, повторяем процесс
    return IsRound(cut);
}

// Добавляем автобус(маршрут) в каталог
static void QueueAddBus(std::string& text, CatalogueCore::TransporCatalogue& cat) {
    // Находим конец для названия автобуса(маршрута)
    auto two_points = text.find_first_of(':');
    // string объект для хранения названия автобуса(маршрута)
    std::string bus_name = text.substr(0, two_points);
    // Создаем пустой контейнер для хранения ссылок на остановки, находящиеся на маршруте(автобусе)
    std::vector<const CatalogueCore::Stop*> stops_for_bus;
    // Сохраняем команду без названия автобуса(маршрута)
    std::string stops_queue = text.substr(two_points + 2, text.size());
    // Проверяем тип проходимого маршрута автобусом и сохраняем знак разделения остановок в команде
    char queue_simbol = IsRound(stops_queue);
    // Итерируемся по команде добавления остановок в саршрут(автобус)
    for (std::string stop;; 
    stops_queue = stops_queue.substr(stops_queue.find_first_of(queue_simbol) + 2, stops_queue.size()))
    {
        // Сохраняем название остановки
        stop = stops_queue.substr(0, stops_queue.find_first_of(queue_simbol) - 1);
        // Добавляем в контейнер ссылки на остановки
        stops_for_bus.push_back(cat.FindStop(stop));
        // При опустошенной команды с добавлением остановок - останавлимаем
        if (stops_queue == stop)
        {
            break;
        }
    }
    // При не кольцевом движении
    if (queue_simbol == '-')
    {
        // Создаем пустой контейнер для хранения обратного маршрута
        std::vector<const CatalogueCore::Stop*> route_back;
        // Вставляем обратный маршрут без начальной для него точки
        route_back.insert(route_back.end(), std::next(stops_for_bus.rbegin()), stops_for_bus.rend());
        // // Вставляем копию в конец оригинального маршрута
        stops_for_bus.insert(stops_for_bus.end(), route_back.begin(), route_back.end());
    }
    // Добавляем автобус(маршрут)
    cat.AddBus({bus_name, stops_for_bus});
}

// Считываем запрос на добавление и обрабатываем в привязанный каталог
void Reader(CatalogueCore::TransporCatalogue& cat)  {
    // Очередь для добавления автобусов(маршрутов)
    std::vector<std::string> bus_queue;
    // Очередь на добавление реальной дистанции(по дорогам) между остановками
    std::map<std::string, std::map<std::string, size_t>> lenght_queue;
    // string объект для количества вводимых команд
    std::string number_of_commands_str;
    // Получаем первую строчку - количество вводимых команд
    std::getline(std::move(std::cin), number_of_commands_str);
    // Переводим string объект для количества вводимых команд в int объект
    int rotations = std::stoi(number_of_commands_str);
    // Итерируем вводимые команды заданное количество раз
    for (int reads = 0 ; reads < rotations; ++reads)
    {
        // string объект для строчки вводимой команды
        std::string queue;
        // Получаем строку с командой
        std::getline(std::move(std::cin), queue);
        // Ищем первый пробел в команде
        auto first_space = queue.find_first_of(' ');
        // Создаем string объект для типа вводимой команды
        std::string command = queue.substr(0, first_space);
        // Отрезаем от строки команды ее тип
        queue = queue.substr(first_space + 1, queue.size());
        // Если тип команды Stop
        if (command == "Stop")
        {
            // Добавляем сущность остановки и создаем объект дистанций для остановок (остановка , {остановка -> расстояние})
            auto stop_to_sto_lenght = QueueAddStop(queue, cat);
            // Итерируемся по дистанциям между остановками
            for (const auto& [stop, lenght] : stop_to_sto_lenght.second)
            {
                // Добавляем в очередь на добавление реальной дистанции(по дорогам)
                lenght_queue[stop_to_sto_lenght.first][stop] = lenght;
            }
        }
        // Если тип команды Bus
        else if (command == "Bus")
        {
            // Добавляем команду в очередь для добавления автобусов(маршрутов)
            bus_queue.push_back(queue);
        }
        // При неккоректной команде пропускаем шаг
        else
        {
            continue;
        }
    }
    // Итерируемся по очереди на добавление реальной дистанции(по дорогам)
    for (auto& [stop, to_stop] : lenght_queue)
    {
        for (auto& [t_stop, lenght] : to_stop)
        {
            // Добавляем реальную дистанцию(по дорогам) для остановок (остановка -> {остановка -> расстояние})
            cat.SetStopToStopDistance(stop, t_stop, lenght);
        }
    }
    // Итерируемся по очереди для добавления автобусов(маршрутов)
    for (std::string& add_bus : bus_queue)
    {
        // Добавляем автобус(маршрут) в каталог
        QueueAddBus(add_bus, cat);
    }
}

}

}