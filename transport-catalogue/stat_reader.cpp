#include "stat_reader.h"

namespace Output
{

namespace detail
{

// Ввыводим информацию о маршруте(автобусе) и печатем ее
void PrintBus(std::string_view bus_name, CatalogueCore::TransporCatalogue& cat) {
    // Получаем ссылку на автобус(маршрут) по заданному имени
    const CatalogueCore::Bus* bus = cat.FindBus(bus_name);
    // Если автобус(маршрут) не найден
    if (bus == nullptr)
    {
        // Печатаем об отсутствии автобуса(маршрута)
        std::cout << "Bus "s << bus_name << ": not found"s << std::endl;
    }
    // Если автобус найден
    else
    {
        // Сохраняем реальную дистанцию(по дорогам) между всеми остановками
        double lenght = cat.RouteDistance(bus);
        // Считаем разницу между реальной дистанцией(по дорогам) и гео дистанции
        double curvature = lenght / cat.GeoDistanceBetweenStops(bus);
        // Печатаем информацию об автобусе(маршруте)
        std::cout << "Bus "s << bus->bus << ": "s << bus->stops_for_bus_.size() <<
        " stops on route, "s << cat.UniqueStopsOnBus(bus) << " unique stops, "s <<
        lenght << " route length, "s << std::setprecision(6) << curvature << " curvature"s 
        << std::endl;
    }
}

// Ввыводим информацию о остановке и печатаем ее
void PrintStop(std::string_view stop_name, CatalogueCore::TransporCatalogue& cat) {
    // Если остановка не найдена
    if (!cat.FindStop(stop_name))
    {
        // Печатаем об отсутствии остановки
        std::cout << "Stop "s << stop_name << ": not found"s << std::endl;
    }
    // Если остановка найдена
    else
    {
        // Сохраняем маршруты(автобусы), проходящие через остановку
        const auto& buses = cat.GetRoutsOnStop(stop_name);
        // Если таких не имеится
        if (buses.empty())
        {
            // Печатаем об отсутствии маршрутов
            std::cout << "Stop "s << stop_name << ": no buses"s << std::endl;
        }
        // Если такие имеются
        else
        {
            // Печатаем информацию о остановке
            std::cout << "Stop "s << stop_name << ": buses"s;
            for (const auto& bus : buses)
            {
                std::cout << " "s << bus;
            }
            std::cout << std::endl;
        }
    }
}

}

// Считываем запрос на обработку и вывод из привязанного каталога
void Reader(CatalogueCore::TransporCatalogue& cat) {
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
        const auto first_space = queue.find_first_of(' ');
        // Создаем string объект для типа вводимой команды
        std::string command = queue.substr(0, first_space);
        // Отрезаем от строки команды ее тип
        queue = queue.substr(first_space + 1, queue.size());
        // Если тип команды Stop
        if (command == "Stop"s)
        {
            // Ввыводим информацию о остановке и печатаем ее
            detail::PrintStop(queue, cat);
        }
        // Если тип команды Bus
        else if (command == "Bus"s)
        {
            // Ввыводим информацию о маршруте(автобусе) и печатем ее
            detail::PrintBus(queue, cat);
        }
        // При неккоректной команде пропускаем шаг
        else
        {
            continue;
        }
    }
}

}